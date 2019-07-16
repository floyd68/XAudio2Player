#pragma once

#include <XAudio2.h>
#include <x3daudio.h>
#include <wrl/client.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <vector>
#include <stdint.h>
#include <memory>
#include <mutex>
#include <thread>

class CAudioStream
{
public:
	CAudioStream(LPCTSTR szPathName);
	~CAudioStream();

	void Restart();
	std::vector<byte> ReadAll();
	std::vector<byte> GetNextBuffer();

	const WAVEFORMATEX& GetOutputWaveFormatEx() const
	{
		return m_waveFormat;
	}

	uint32_t GetMaxStreamLengthInBytes() const
	{
		return m_maxStreamLengthInBytes;
	}

private:
	Microsoft::WRL::ComPtr<IMFSourceReader> m_reader;
	WAVEFORMATEX							m_waveFormat;
	uint32_t								m_maxStreamLengthInBytes;
};

class CVoice
{
public:
	CVoice(IXAudio2SourceVoice* pVoice) : m_pVoice(pVoice) {}
	~CVoice();

	virtual void Play() = NULL;

protected:
	IXAudio2SourceVoice* m_pVoice;
};

const int STREAM_BUFFER_COUNT = 3;

class CStreamVoice : public CVoice
{
public:
	CStreamVoice(IXAudio2SourceVoice* pVoice, std::unique_ptr<CAudioStream>&& pStream);
	~CStreamVoice();

	void Play() override;
	void Stop();

	bool IsPlaying() { return m_bPlaying; }
	bool IsBufferAvail();
	bool QueBuffer();

private:
	std::unique_ptr<CAudioStream>	m_pStream;
	std::vector<byte>				m_buffers[STREAM_BUFFER_COUNT];
	int								m_nCurrentBuffer;
	bool							m_bPlaying;
};

class CBufferedVoice : public CVoice
{
public:
	CBufferedVoice(IXAudio2SourceVoice* pVoice, std::vector<byte>&& data);

	void Play() override;

private:
	std::vector<byte> m_data;
};

class CAudioEngine
{
public:
	CAudioEngine(float fSpeedOfSound);
	~CAudioEngine();

	void Update();
	void StartEngine();
	void StopEngine();

	CVoice* CreateVoice(LPCTSTR lpszFileName);
	CVoice* CreateStreamVoice(LPCTSTR lpszFileName);

private:
	Microsoft::WRL::ComPtr<IXAudio2> m_pXAudio2;
	IXAudio2MasteringVoice* m_pMasterVoice;
	X3DAUDIO_HANDLE m_hX3DAudio;
	
	std::thread m_thread;

	std::mutex m_mutex;
	std::vector <std::shared_ptr<CStreamVoice>> m_vecStreamVoices;
	HANDLE m_hKillThread;
};