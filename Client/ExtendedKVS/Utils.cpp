#include "Utils.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <vector>
#include <clocale>
#include <cwchar>
#endif

#include <string>

namespace kvs{

std::string MultibyteToUTF8(const std::string& text){
#ifdef _WIN32
    // Multibyte to Wide
    int wideLength = MultiByteToWideChar(CP_THREAD_ACP, 0, text.c_str(), -1, NULL, 0);
    wchar_t* wideBuffer = new wchar_t[wideLength];
    if (0 == MultiByteToWideChar(CP_THREAD_ACP, 0, text.c_str(), -1, wideBuffer, wideLength)){
        delete wideBuffer;
        return 0;
    }

    // Wide to UTF8
    int utf8Length = WideCharToMultiByte(CP_UTF8, 0, wideBuffer, -1, NULL, 0, NULL, NULL);
    char* utf8Buffer = new char[utf8Length];
    if(0 == WideCharToMultiByte(CP_UTF8, 0, wideBuffer, -1, utf8Buffer, utf8Length, NULL, NULL)){
        delete wideBuffer;
        delete utf8Buffer;
        return 0;
    }

    std::string strUTF8(utf8Buffer);
    delete wideBuffer;
    delete utf8Buffer;
#else
    // get initial locale like "C"
    std::string prevLocale = std::setlocale(LC_ALL, NULL);

    // set default user-locale like "Japanese_Japan.932"
    setlocale(LC_ALL, "");

    std::mbstate_t state = std::mbstate_t();

    // Multibyte to Wide
    const char* mbstr = text.c_str();
    std::size_t wideLength = std::mbsrtowcs(NULL, &mbstr, 0, &state) + 1;
    std::vector<wchar_t> wideCharacters(wideLength);
    std::mbsrtowcs(&wideCharacters[0], &mbstr, wideCharacters.size(), &state);

    // Wide to UTF8
    std::setlocale(LC_ALL, ".utf8");
    const wchar_t* wstr = &wideCharacters[0];
    std::size_t utf8Length = std::wcsrtombs(nullptr, &wstr, 0, &state) + 1;
    std::vector<char> utf8bytes(utf8Length);
    std::wcsrtombs(&utf8bytes[0], &wstr, utf8bytes.size(), &state);

    std::string strUTF8(&utf8bytes[0]);

    // restore initial locale
    std::setlocale(LC_ALL, prevLocale.c_str());
#endif
    return strUTF8;
}

} // kvs
