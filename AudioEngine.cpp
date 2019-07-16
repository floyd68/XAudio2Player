#include "AudioEngine.h"

#include "error.h"


CAudioStream::CAudioStream(LPCTSTR szPathName)
	: m_maxStreamLengthInBytes(0)
	, m_waveFormat{}
{
	Microsoft::WRL::ComPtr<IMFMediaType> outputMediaType;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

	Microsoft::WRL::ComPtr<IMFAttributes> lowLatencyAttribute;
	ThrowIfFail(
		MFCreateAttributes(&lowLatencyAttribute, 1)
	);

	ThrowIfFail(lowLatencyAttribute->SetUINT32(MF_LOW_LATENCY, TRUE));
	ThrowIfFail(MFCreateSourceReaderFromURL(szPathName, lowLatencyAttribute.Get(), &m_reader));
	ThrowIfFail(MFCreateMediaType(&mediaType));

	ThrowIfFail(mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));

	ThrowIfFail(mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));

	ThrowIfFail(m_reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, mediaType.Get()));

	ThrowIfFail(m_reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &outputMediaType));

	uint32_t formatByteCount = 0;
	WAVEFORMATEX* waveFormat;

	ThrowIfFail(MFCreateWaveFormatExFromMFMediaType(outputMediaType.Get(), &waveFormat, &formatByteCount));

	CopyMemory(&m_waveFormat, waveFormat, sizeof(m_waveFormat));
	CoTaskMemFree(waveFormat);

	PROPVARIANT var;
	ThrowIfFail(m_reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var));

	LONGLONG duration = var.uhVal.QuadPart;
	double durationInSeconds = (duration / (double)(10000 * 1000));
	m_maxStreamLengthInBytes = (uint32_t)(durationInSeconds * m_waveFormat.nAvgBytesPerSec);
}

CAudioStream::~CAudioStream()
{

}

std::vector<byte> CAudioStream::GetNextBuffer()
{
	std::vector<byte> resultData;

	Microsoft::WRL::ComPtr<IMFSample> sample;
	Microsoft::WRL::ComPtr<IMFMediaBuffer> mediaBuffer;

	uint8_t* audioData = nullptr;
	uint32_t sampleBufferLength = 0;
	uint32_t flags = 0;

	ThrowIfFail(m_reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, reinterpret_cast<DWORD*>(&flags), nullptr, &sample));

	// End of stream
	if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
	{
		return resultData;
	}
	if (sample == nullptr)
		throw std::exception("Fail");

	ThrowIfFail(sample->ConvertToContiguousBuffer(&mediaBuffer));

	ThrowIfFail(mediaBuffer->Lock(&audioData, nullptr, reinterpret_cast<DWORD*>(&sampleBufferLength)));

	//
	// Prepare and return the resultant array of data
	//
	resultData.resize(sampleBufferLength);
	CopyMemory(&resultData[0], audioData, sampleBufferLength);

	// Release the lock
	ThrowIfFail(mediaBuffer->Unlock());

	return resultData;
}

std::vector<byte> CAudioStream::ReadAll()
{
	std::vector<byte> resultData;

	// Make sure stream is set to start
	// Restart();

	for (;;)
	{
		std::vector<byte> nextBuffer = GetNextBuffer();
		if (nextBuffer.size() == 0)
		{
			break;
		}

		// Append the new buffer to the running total
		size_t lastBufferSize = resultData.size();
		resultData.resize(lastBufferSize + nextBuffer.size());
		CopyMemory(&resultData[lastBufferSize], &nextBuffer[0], nextBuffer.size());
	}

	return resultData;
}

void CAudioStream::Restart()
{
	PROPVARIANT var;
	ZeroMemory(&var, sizeof(var));
	var.vt = VT_I8;

	ThrowIfFail(m_reader->SetCurrentPosition(GUID_NULL, var));
}


CVoice::~CVoice()
{
	m_pVoice->DestroyVoice();
}

CStreamVoice::CStreamVoice(IXAudio2SourceVoice* pVoice, std::unique_ptr<CAudioStream>&& pStream)
	: CVoice(pVoice)
	, m_pStream(std::move(pStream))
	, m_nCurrentBuffer(0)
	, m_bPlaying( false )
{
}

CStreamVoice::~CStreamVoice()
{
	m_pStream.reset();
}


bool CStreamVoice::IsBufferAvail()
{
	XAUDIO2_VOICE_STATE state;
	m_pVoice->GetState(&state);
	return state.BuffersQueued < STREAM_BUFFER_COUNT;
}

bool CStreamVoice::QueBuffer()
{
	m_buffers[m_nCurrentBuffer] = m_pStream->GetNextBuffer();

	XAUDIO2_BUFFER buf = { 0 };

	buf.AudioBytes = static_cast<uint32_t>(m_buffers[m_nCurrentBuffer].size());
	buf.pAudioData = &m_buffers[m_nCurrentBuffer][0];

	ThrowIfFail(m_pVoice->SubmitSourceBuffer(&buf));

	m_nCurrentBuffer = ++m_nCurrentBuffer % STREAM_BUFFER_COUNT;

	return true;
}

void CStreamVoice::Play()
{
	m_nCurrentBuffer = 0;

	QueBuffer();
	
	m_pVoice->Start(0);

	m_bPlaying = true;
}

void CStreamVoice::Stop()
{
	m_pVoice->Stop();
	m_bPlaying = false;
}

CBufferedVoice::CBufferedVoice(IXAudio2SourceVoice* pVoice, std::vector<byte>&& data)
	: CVoice(pVoice)
	, m_data(std::move(data))
{
}




void CBufferedVoice::Play()
{
	XAUDIO2_BUFFER buf = { 0 };

	buf.AudioBytes = static_cast<uint32_t>(m_data.size());
	buf.pAudioData = &m_data[0];

	ThrowIfFail(m_pVoice->SubmitSourceBuffer(&buf));
	m_pVoice->Start(0);
}




CAudioEngine::CAudioEngine(float fSpeedOfSound)
	: m_pMasterVoice(nullptr)
{
	ThrowIfFail(MFStartup(MF_VERSION));
	ThrowIfFail(XAudio2Create(&m_pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR));
	ThrowIfFail(m_pXAudio2->CreateMasteringVoice(&m_pMasterVoice));

	DWORD dwChannelMask;
	ThrowIfFail(m_pMasterVoice->GetChannelMask(&dwChannelMask));

	ThrowIfFail(X3DAudioInitialize(dwChannelMask, fSpeedOfSound, m_hX3DAudio));

	m_hKillThread = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_thread = std::thread([this]()
		{
			while(true)
			{
				if (WaitForSingleObject(m_hKillThread, 0) == WAIT_OBJECT_0)
					return;
				else
				{
					std::lock_guard<std::mutex> lock(m_mutex);

					for (auto pVoice : m_vecStreamVoices)
					{
						if (pVoice->IsPlaying())
						{
							while (pVoice->IsBufferAvail())
								pVoice->QueBuffer();
						}
					}
				}
				Sleep(1);
			}
		}
	);
}

CAudioEngine::~CAudioEngine()
{
	StopEngine();
	SetEvent(m_hKillThread);
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_vecStreamVoices.clear();
	}

	m_pMasterVoice->DestroyVoice();
	m_thread.join();

	CloseHandle(m_hKillThread);

	m_pXAudio2.Reset();

	MFShutdown();
}


void CAudioEngine::StartEngine()
{
	m_pXAudio2->StartEngine();
}

void CAudioEngine::StopEngine()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		for (auto pVoice : m_vecStreamVoices)
			pVoice->Stop();
	}
	m_pXAudio2->StopEngine();
}

void CAudioEngine::Update()
{
}

CVoice* CAudioEngine::CreateVoice(LPCTSTR lpszFileName)
{
	std::unique_ptr<CAudioStream> pStream = std::make_unique<CAudioStream>(lpszFileName);

	IXAudio2SourceVoice* pSourceVoice;
	ThrowIfFail(m_pXAudio2->CreateSourceVoice(&pSourceVoice, &pStream->GetOutputWaveFormatEx()));

	return new CBufferedVoice(pSourceVoice, pStream->ReadAll());
}

CVoice* CAudioEngine::CreateStreamVoice(LPCTSTR lpszFileName)
{
	std::unique_ptr<CAudioStream> pStream = std::make_unique<CAudioStream>(lpszFileName);

	IXAudio2SourceVoice* pSourceVoice;
	ThrowIfFail(m_pXAudio2->CreateSourceVoice(&pSourceVoice, &pStream->GetOutputWaveFormatEx()));

	auto pStreamVoice = std::make_shared<CStreamVoice>(pSourceVoice, std::move(pStream));

	{
		std::lock_guard<std::mutex> lock(m_mutex);

		m_vecStreamVoices.push_back(pStreamVoice);
	}

	return pStreamVoice.get();
}