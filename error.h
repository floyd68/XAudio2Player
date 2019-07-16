#pragma once
#include <comdef.h>
#include <exception>
#include <string>
#include <stdlib.h>


inline std::string hresult_to_string(HRESULT hr)
{
	_com_error err(hr);

	auto wszError = err.ErrorMessage();

	char szTemp[256];
	size_t sz;
	wcstombs_s(&sz, szTemp, 256, wszError, 256);

	return szTemp;
}

class com_error : public std::exception
{
public:
	com_error(HRESULT hr)
		: std::exception( hresult_to_string(hr).c_str() )
	{
	}
};

inline void ThrowIfFail(HRESULT hr)
{
	if (FAILED(hr))
		throw com_error(hr);
}