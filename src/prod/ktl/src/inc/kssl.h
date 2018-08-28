/*++

    (c) 2010 by Microsoft Corp. All Rights Reserved.

    Description:
      Describes a SSL wrapper for network security purposes.

    History:
      leikong         Oct-2011.........Adding implementation
--*/

#pragma once

//
// KSslTransport
//
class KSslTransport : public KSspiTransport
{
public:
    typedef KSharedPtr<KSslTransport> SPtr;

    static NTSTATUS Initialize(
        _Out_ ITransport::SPtr& Transport,
        _In_ KAllocator& alloc,
        ITransport::SPtr const & innerTransport,
        KSspiCredential::SPtr const & clientCredential,
        KSspiCredential::SPtr const & serverCredential,
        AuthorizationContext::SPtr const & authorizationContext);

    NTSTATUS CreateSspiSocket(
        ISocket::SPtr && innerSocket,
        KSspiContext::UPtr && sspiContext,
        __out ISocket::SPtr & sspiSocket);

    // SSL has its own framing/record protocol, so it doesn't need extra framing.
    // This method will simply return the input innerSocket.
    NTSTATUS CreateFramingSocket(
        ISocket::SPtr && innerSocket,
        __out ISocket::SPtr & framingSocket);

    NTSTATUS CreateSspiClientContext(
        _In_opt_ PWSTR targetName,
        _Out_ KSspiContext::UPtr & sspiContext);

    NTSTATUS CreateSspiServerContext(_Out_ KSspiContext::UPtr & sspiContext);

private:
    KSslTransport(
        ITransport::SPtr const & innerTransport,
        KSspiCredential::SPtr const & clientCredential,
        KSspiCredential::SPtr const & serverCredential,
        AuthorizationContext::SPtr const & authorizationContext);
};

//
// KSslSocket
//
class KSslSocket : public KSspiSocket
{
    friend class KSslTransport;
    friend class KSslSocketReadOp;
    friend class KSslSocketWriteOp;

public:
    typedef KSharedPtr<KSslSocket> SPtr;

    ~KSslSocket();

    NTSTATUS CreateReadOp(_Out_ ISocketReadOp::SPtr& newOp);
    NTSTATUS CreateWriteOp(_Out_ ISocketWriteOp::SPtr& newOp);

private:
    KSslSocket(
        KSspiTransport::SPtr && sspiTransport,
        ISocket::SPtr && innerSocket,
        KSspiContext::UPtr && sspiContext);

    NTSTATUS OnNegotiationSucceeded();

    NTSTATUS SetupBuffers();

    // There may have been application data received during SSL negotiation
    void CopyExtraDataReceivedDuringNegotiation();

private:
    SecPkgContext_StreamSizes _streamSizes;
};

//
// KSslContext
//
class KSslContext : public KSspiContext
{
    friend class KSslTransport;

public:
    ~KSslContext();

    // SSL has their own message framing protocol. We need to save the extra data we may have read during
    // security negotiation and pass it onto the socket instance generated by SSPI socket accept/connect
    KMemRef ExtraData() const;

private:
    KSslContext(
        _In_ KAllocator& alloc,
        _In_ PCredHandle credHandle,
        _In_opt_ PWSTR targetName,
        bool accept,
        ULONG requirement,
        ULONG receiveBufferSize);

    ULONG TargetDataRep();

    PSecBufferDesc InputSecBufferDesc();

    void SetInputSecBufferDesc(PVOID inputAddress, ULONG inputLength);

    ULONG RemainingInputLength();

    void SaveExtraInputIfAny();

    SecBuffer _inputSecBuffer[2];
    SecBufferDesc _inputSecBufferDesc;

    KMemRef _extraData;
};

class KSslSocketReadOp : public KSspiSocketReadOp
{
    friend class KSslSocket;

    KSslSocketReadOp(KSslSocket::SPtr && sslSocket);

    void DecryptMessage();

    void MoveExtraCiphertextToStart(SecBuffer const * buffers);
};

class KSslSocketWriteOp : public KSspiSocketWriteOp
{
    friend class KSslSocket;

    KSslSocketWriteOp(KSslSocket::SPtr && sslSocket);

    NTSTATUS EncryptMessage();
};