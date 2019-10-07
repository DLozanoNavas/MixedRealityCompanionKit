// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Media.NetworkMediaSink.g.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>

namespace winrt::RealtimeStreaming::Media::implementation
{
        struct NetworkMediaSink : NetworkMediaSinkT<NetworkMediaSink, IMFMediaSink, IMFClockStateSink>
        {
        public:
            NetworkMediaSink(_In_ RealtimeStreaming::Network::Connection connection);
            virtual ~NetworkMediaSink();

            // IMediaExtension
            void SetProperties(Windows::Foundation::Collections::IPropertySet const& configuration) {};

            // IMFMediaSink methods
            IFACEMETHOD(GetCharacteristics)(
                _Out_ DWORD* pdwCharacteristics);
            IFACEMETHOD(AddStreamSink)(
                _In_ DWORD dwStreamSinkIdentifier,
                _In_opt_ IMFMediaType* pMediaType,
                _COM_Outptr_opt_ IMFStreamSink** ppStreamSink);
            IFACEMETHOD(RemoveStreamSink)(
                _In_ DWORD dwStreamSinkIdentifier);
            IFACEMETHOD(GetStreamSinkCount)(
                _Out_ DWORD* pcStreamSinkCount);
            IFACEMETHOD(GetStreamSinkByIndex)(
                _In_ DWORD dwIndex, 
                _COM_Outptr_opt_ IMFStreamSink** ppStreamSink);
            IFACEMETHOD(GetStreamSinkById)(
                _In_ DWORD dwIdentifier, 
                _COM_Outptr_opt_ IMFStreamSink** ppStreamSink);
            IFACEMETHOD(SetPresentationClock)(
                _In_opt_ IMFPresentationClock* pPresentationClock);
            IFACEMETHOD(GetPresentationClock)(
                _COM_Outptr_opt_ IMFPresentationClock** ppPresentationClock);
            IFACEMETHOD(Shutdown)();

            // IMFClockStateSink methods
            IFACEMETHOD(OnClockStart)(
                _In_ MFTIME hnsSystemTime, 
                _In_ LONGLONG llClockStartOffset);
            IFACEMETHOD(OnClockStop)(
                _In_ MFTIME hnsSystemTime);
            IFACEMETHOD(OnClockPause)(
                _In_ MFTIME hnsSystemTime);
            IFACEMETHOD(OnClockRestart)(
                _In_ MFTIME hnsSystemTime);
            IFACEMETHOD(OnClockSetRate)(
                _In_ MFTIME hnsSystemTime, 
                _In_ float flRate);

            // NetworkMediaSink
            void StartNetwork();
            void StopNetwork();

            winrt::event_token NetworkMediaSink::Closed(Windows::Foundation::EventHandler<bool> const& handler);
            void NetworkMediaSink::Closed(winrt::event_token const& token);

        private:
            winrt::fire_and_forget SendStreamReady();
            winrt::fire_and_forget SendStreamStopped();
            winrt::fire_and_forget SendDescription();

            void OnDataReceived(
                //_In_ Network::Connection const& sender,
                _In_ IInspectable const& sender,
                _In_ Network::DataBundleArgs const& args);

            HRESULT HandleError(_In_ HRESULT hr);

            HRESULT CheckShutdown()
            {
                NULL_CHK_HR(m_connection, MF_E_SHUTDOWN);

                //IFT(_cStreamsEnded == 0 ? S_OK : MF_E_SHUTDOWN);

                return S_OK;
            }

        private:
            slim_mutex m_lock;

            LONGLONG m_llStartTime;
            winrt::com_ptr<IMFPresentationClock> m_presentationClock;  // Presentation clock.

            std::vector<winrt::com_ptr<IMFStreamSink> > m_streams;

            RealtimeStreaming::Network::Connection m_connection{ nullptr };
            winrt::event_token m_bundleReceivedEventToken;

            winrt::event<Windows::Foundation::EventHandler<bool>> m_evtClosed;
        };
}

namespace winrt::RealtimeStreaming::Media::factory_implementation
{
    struct NetworkMediaSink : NetworkMediaSinkT<NetworkMediaSink, implementation::NetworkMediaSink>
    {
    };
}
