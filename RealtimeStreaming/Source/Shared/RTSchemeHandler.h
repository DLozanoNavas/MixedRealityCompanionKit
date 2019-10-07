// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "Media.RTSchemeHandler.g.h"

namespace winrt::RealtimeStreaming::Media::implementation
{
    struct RTSchemeHandler : RTSchemeHandlerT<RTSchemeHandler, IMFSchemeHandler>
    {
        public:
            RTSchemeHandler();

            //RTSchemeHandler
            RealtimeStreaming::Network::Connection RTSchemeHandler::DataConnection();
            void RTSchemeHandler::DataConnection(RealtimeStreaming::Network::Connection const& connection);

            // IMediaExtension
            void SetProperties(Windows::Foundation::Collections::IPropertySet const& configuration);

            // IMFSchemeHandler
            IFACEMETHOD(BeginCreateObject) (
                _In_ LPCWSTR pwszURL,
                _In_ DWORD dwFlags,
                _In_ IPropertyStore* pProps,
                _COM_Outptr_opt_  IUnknown** ppIUnknownCancelCookie,
                _In_ IMFAsyncCallback* pCallback,
                _In_ IUnknown* punkState);

            IFACEMETHOD(EndCreateObject) (
                _In_ IMFAsyncResult* pResult,
                _Out_ MF_OBJECT_TYPE* pObjectType,
                _COM_Outptr_ IUnknown** ppObject);

            IFACEMETHOD(CancelObjectCreation) (
                _In_ IUnknown* pIUnknownCancelCookie);

        private:
            RealtimeStreaming::Network::Connection m_connection{ nullptr };
        };
}

namespace winrt::RealtimeStreaming::Media::factory_implementation
{
    struct RTSchemeHandler : RTSchemeHandlerT<RTSchemeHandler, implementation::RTSchemeHandler>
    {
    };
}
