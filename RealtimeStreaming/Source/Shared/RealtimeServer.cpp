// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"
#include "RealtimeServer.h"
#include "MediaUtils.h"
#include "NetworkMediaSink.h"

using namespace winrt;
using namespace RealtimeStreaming::Media::implementation;
using namespace Windows::Media::MediaProperties;

RealtimeServer::RealtimeServer(
    RealtimeStreaming::Network::Connection connection,
    GUID inputMediaType,
    MediaEncodingProfile mediaEncodingProperties)
    : m_spMediaEncodingProfile(mediaEncodingProperties)
{
    Log(Log_Level_Info, L"RealtimeServer::RealtimeServer() - Tid:%d \n", GetCurrentThreadId());

    // TODO: Currently hardcoded to not accept audio
    m_spMediaEncodingProfile.Audio(nullptr);
    m_spMediaEncodingProfile.Container(nullptr);

    // create the custom network sink
    m_spNetworkMediaSink = winrt::make<Media::implementation::NetworkMediaSink>(connection);

    // Create sink writer which will write to our custom network sink
    com_ptr<IMFSinkWriter> spSinkWriter;
    com_ptr<IMFAttributes> spAttributes;
    IFT(MFCreateAttributes(spAttributes.put(), 1));
    IFT(spAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));

    IFT(MFCreateSinkWriterFromMediaSink(m_spNetworkMediaSink.as<IMFMediaSink>().get(),
        spAttributes.get(),
        spSinkWriter.put()));

    // Set the output media type.
    com_ptr<IMFMediaType>  spMediaTypeOut;
    IFT(MFCreateMediaTypeFromProperties(
        winrt::get_unknown(m_spMediaEncodingProfile.Video()),
        spMediaTypeOut.put()));

    DWORD sinkWriterStream;
    IFT(spSinkWriter->AddStream(spMediaTypeOut.get(), &sinkWriterStream));
    m_sinkWriterStream = sinkWriterStream;

    // Set the input media type.
    com_ptr<IMFMediaType>  spMediaTypeIn;
    IFT(MFCreateMediaType(spMediaTypeIn.put()));

    UINT32 videoWidth = m_spMediaEncodingProfile.Video().Width();
    UINT32 videoHeight = m_spMediaEncodingProfile.Video().Height();

    IFT(spMediaTypeIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
    IFT(spMediaTypeIn->SetGUID(MF_MT_SUBTYPE, inputMediaType));
    IFT(spMediaTypeIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
    IFT(MFSetAttributeSize(spMediaTypeIn.get(), MF_MT_FRAME_SIZE, videoWidth, videoHeight));
    IFT(MFSetAttributeRatio(spMediaTypeIn.get(), MF_MT_FRAME_RATE, VIDEO_FPS, 1));
    IFT(MFSetAttributeRatio(spMediaTypeIn.get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1));

    IFT(spSinkWriter->SetInputMediaType(m_sinkWriterStream, spMediaTypeIn.get(), NULL));

    // Tell the sink writer to start accepting data.
    IFT(spSinkWriter->BeginWriting());

    m_mediaInputFormat = inputMediaType;

    m_spSinkWriter = spSinkWriter;

    m_spNetworkMediaSink.StartNetwork();
}

RealtimeServer::~RealtimeServer()
{
    Shutdown();
}

_Use_decl_annotations_
void RealtimeServer::Shutdown()
{
    if (m_spSinkWriter == nullptr)
    {
        return;
    }

    IFT(m_spSinkWriter->Finalize()); // Close sinkwriter

    m_spSinkWriter = nullptr; // release sinkwriter
    m_spNetworkMediaSink = nullptr;
}

_Use_decl_annotations_
void RealtimeServer::WriteFrame(uint32_t bufferSize, 
    array_view<uint8_t const> bufferArrayView)
{
    Log(Log_Level_Verbose, L"RealtimeServer::WriteFrame() - BufferSize=%d \n", bufferSize);

    NULL_THROW(m_spSinkWriter);

    const byte* pBuffer = bufferArrayView.data();
    NULL_THROW(pBuffer);

    // Create MediaBuffer
    com_ptr<IMFMediaBuffer> spBuffer;

    auto videoProps = m_spMediaEncodingProfile.Video();

    int bpp = GetBytesPerPixel(m_mediaInputFormat);
    if (bpp != -1 && (videoProps.Width() * videoProps.Height() * bpp != bufferSize))
    {
        Log(Log_Level_Verbose, L"RealtimeServer::WriteFrame() - Invalid buffer size w=%d - h=%d \n", videoProps.Width(), videoProps.Height());
    }

    IFT(CreateIMFMediaBuffer(videoProps.Width(),
        videoProps.Height(),
        bufferSize,
        (BYTE*)pBuffer,
        spBuffer.put()));

    // Create a media sample and add the buffer to the sample.
    com_ptr<IMFSample> spSample;
    IFT(MFCreateSample(spSample.put()));
    IFT(spSample->AddBuffer(spBuffer.get()));

    // Set the time stamp and the duration.
    IFT(spSample->SetSampleTime(rtStart));
    IFT(spSample->SetSampleDuration(VIDEO_FRAME_DURATION));

    // Send the sample to the Sink Writer.
    IFT(m_spSinkWriter->WriteSample(m_sinkWriterStream, spSample.get()));

    rtStart += VIDEO_FRAME_DURATION;
}

_Use_decl_annotations_
VideoEncodingProperties RealtimeServer::VideoProperties()
{
    return m_spMediaEncodingProfile.Video();
}

_Use_decl_annotations_
int RealtimeServer::GetBytesPerPixel(GUID inputMediaType)
{
    auto index = MF_GUIDs_to_BPP.find(inputMediaType);
    if (index != MF_GUIDs_to_BPP.end())
    {
        return index->second;
    }

    return - 1;
}