#include "FilterInformation.h"
#include "PBVRFileInformation/UnstructuredPfi.h"

class FileFormatReader {
public:
	/*===========================================================================*/
	/*
	*  @brief  変換前のファイルパスを読み込み必要な情報をFilterInformationListクラスに格納する関数
	*  @param  std::string file_path 変換前のファイルパス
	*  @return FilterInformationList 必要な情報が格納されているFilterInformationListクラス
	*/
	/*===========================================================================*/
	FilterInformationList ConvertFilterInformationList( std::string  file_path );
	/*===========================================================================*/
	/*
	*  @brief  変換前のファイルパスを読み込みcvt::UnstructuredPfiクラスの連想配列を作成する関数
				cvt::UnstructuredPfiにはPfiファイルに記述するために必要な情報が格納されている
	*  @param  std::string file_path 変換前のファイルパス
	*  @return std::unordered_map<int, cvt::UnstructuredPfi> 変換前ファイルの情報が格納されたcvt::UnstructuredPfiクラスの連想配列
	*/
	/*===========================================================================*/
	std::unordered_map<int, cvt::UnstructuredPfi> ConvertUnstructuredPfiMap( std::string  file_path );
};