#include "Utils.h"

/**
     * @brief OS に応じてパスをネイティブ形式に変換する
     *
     * - Windows ではスラッシュ (`/`) をバックスラッシュ (`\`) に変換する。
     * - Unix 系（Linux/macOS）ではそのまま。
     *
     * @param path 変換前のパス
     * @return 変換後の OS 固有のパス（std::string 型）
     *
     * @note
     * QString を std::string に変換する際、`qstr.toStdString()` は UTF-8 固定。
     * しかし、KVS が期待するエンコーディングと異なる可能性があるため使用しない。
     * 代わりに `toLocal8Bit().constData()` を用いて、OS のロケールに基づいた
     * エンコーディングで変換する。
     * - Windows: Shift_JIS
     * - Linux/macOS: UTF-8
     */
std::string Utils::toNativePath(const QString& path)
{
    QString nativePath = path;  // コピーを作成
#ifdef Q_OS_WIN
    return nativePath.replace("/", "\\").toLocal8Bit().constData(); // Windows の場合
#else
    return nativePath.toLocal8Bit().constData(); // Unix 系 (Linux/macOS)
#endif
}
