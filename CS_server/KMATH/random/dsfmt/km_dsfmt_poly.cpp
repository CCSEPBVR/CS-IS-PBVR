
#include "km_dsfmt_poly.h"

#if DSFMT_MEXP == 521

int km_dsfmt_period = 521;

std::string km_dsfmt_poly = 
  "148e2d345750a90fa0b09763ecb70053a2490bdaedef3aa8fa"
  "0c85fc079aa729496743b5d59b0d7a9cea17df2a67dd59ea7a"
  "3bb1beef4c3d1ec90221d62cc371642710111";

#elif DSFMT_MEXP == 1279

int km_dsfmt_period = 1279;

std::string km_dsfmt_poly = 
  "14ca4b14ed4dab59798009986994e69dd13077f1a9e440dbf8"
  "b8ed606eeed432fd13b37e501ad87f3078e483cb19c2bd6c4a"
  "b7c088a032491825bec1cb67695ca591a4f2b9d9bef559f1f0"
  "df86b93107b474c08f65655a38c4dcba34722136b0ce066fbc"
  "37a441267541b17a44a7815c27069b930fb28d4d21b55d9d64"
  "8187a8c361119d3c42dcc534812c607b48fead6d3b4443aad6"
  "e11b62511ea1520fa1df6f6920f9df8b94986d91e5e21";

#elif DSFMT_MEXP == 2203

int km_dsfmt_period = 2203;

std::string km_dsfmt_poly = 
  "1407b8c6688f1c0775fea3883299244920796c03596070e493"
  "f8a2466b86e7415739c0304943507214066639931675ff67f0"
  "b7aa0d5a59fcd60b35a658e3b0718ced7e1d63afa917bd3f2e"
  "0ba47059a48846f24d2145184e41b4b552020209d27237e6af"
  "dd3bfa9cfc958b26802191881eaf9240b1543e395e50247e31"
  "992d5dac592c5780c5ab642461fd9aaaf9fdf04f0c2a7bce50"
  "d70550d2e8269ef57a1bc1b14aa3108d0555d03299d068fa02"
  "520574ddc5c4b121037e2a5f9d0142fc871fc146b947aa81ad"
  "cb6d32f0d801094c8de6573c027a0a0418f7fff8f2c85e16cd"
  "1086d5dce3b93d546a4b4bcea0864eb26213230195583f6eb4"
  "9fccb02659a558b7bcfd09f218e31603565151444010105555"
  "50100001";

#elif DSFMT_MEXP == 4253

int km_dsfmt_period = 4253;

std::string km_dsfmt_poly = 
  "11110a3af24f296024e376f219a750d04bac6afd10cff9221e"
  "5d6bbf24b7970687dabb371262579d55820616b7eef3ad2979"
  "f44619d0d21a422c56968795ddba6f92bca413c1787299e56c"
  "271dc053d10e7675b8973938f1851ca195e3457c6022d6f894"
  "977e69be85838bb2ef0266ad19311b944de9a7b9bcbbe94534"
  "21c723d84a3832bbebc316ec1a951edf54d1e0ff7de2e2fc6a"
  "c5d72439e56d2e4caa0f95ab3243ee2b6b89144dc92bb542c6"
  "1c3611a625e33c4cbc67bdfd5012ce34e332eeecf028a105fc"
  "e45bb220295362ccf8aa176c92b99383c37faa63defba250bf"
  "1cb32251f9ad36a8722ede5baa33d8b5a16c577777b9493f4e"
  "4c021a8d5d3ce07b54eddfd3a796997a2a65decd4b6890b7bd"
  "c5bf139d2ffbfa6cfc4636932ae195c937c9a00770a5146839"
  "461315adabab10511697ab431275373b47563da464fa3a4f4c"
  "4378dc62fae3b56e269df2e8a21a23fc3ef78bb52fabea4737"
  "7967bdfd8b238cb0dc55fa5ff974c469d926a8108b4b27ec8a"
  "8f863f3f2a2e517d3fe4ab86af410c0bb85085d285edb9d5e5"
  "48c6e1f5db0c77167bc9f2d8c66322dc0897e1bd225632aa4e"
  "d55987883f062daa6020355986826f65f7d672d18d26f66352"
  "b2e46b86c1c39bb1ca976b353d043c6cde4ba4741ad83e7969"
  "ae374ef96b4aad17d6eaa95cc6fc99d609bd78c60eb0695794"
  "397e9c7e9e8bcbc343e3e3e6ee6eeebbaaa00aaabafffff777"
  "7777777fffffaaaaa000001";

#elif DSFMT_MEXP == 11213

int km_dsfmt_period = 11213;

std::string km_dsfmt_poly = 
  "101100115cf68ab39e5737fb551005d2d5e60fba461aa514f0"
  "56fe3dea7932881ec502691a4f73cee271a369a9c6952890ba"
  "8150ad73d12700639ab4a95b024b5d4e5959ff9028ab91c737"
  "cfc42ef9f71f1e1bba39b04d120cd97cd02964bcebe59278ed"
  "5ec52a4b8d7dfd3a61771ea2dd1f98290a475aecb8a5f64e4a"
  "03e9cb726c3542b50a25f4f16431b9da54aeeb46d7d787c477"
  "314555e4b86ba611b6f8cf8647cdc95496ad7521a503b685bf"
  "37a7ce5960e96ceacd6870b74ea00b724700cd2daad58d4d0b"
  "d84d0653af9161f7d57fe7fc893652fc0b9e24e93c42b8b1a4"
  "f1cdd3425c8087abfae9c40aafd058ae58c256ac455947d8cf"
  "123968c992832f4836a2fde99e102c5ea3ea9b31008e732fee"
  "e730c5a58123492433732bcff38749ba00565163daea2f7eea"
  "73668b6a3293935ab58ea5418a1dd3f05b651f85d8c9c58903"
  "2a668901bcc0935e2e71ad93ebd2be3733af33ad0d90a2f82f"
  "35ab264bd86755737a468d93f37ec6c9d19b7fed0d1c5115e5"
  "6559ef544a6bbe1273054a2f9c35d6179357f3ba3dae1fc13b"
  "513319f2260fd7d942786387edadc17ddc5b99f3b89d1911b2"
  "ea2bea7e8277905ed74eb50f52735a92fdfd6d150ad1ad0dc1"
  "0a6183265384a831da640c0c97f4132fe9ef9994f7493c0033"
  "f75cb55162e683cd551388c81716706a8ce57801c291778257"
  "d50fbeeb4c4f04f1088eb8cdf2e8dd9e491717f7f81f16d503"
  "db870efeb8136503c67928f5c45ed2a8610c9a91b507b1e8f6"
  "a1c51b58a2b5869f71e84daf0a0833e600eca6eb41a3e86e4d"
  "8791bbe30977be349f08ff577c73dcdc2a238e4d6e648e4c35"
  "5bc82429b23d19dc299c5e5eea3974e056b35e9dddd4f0ac08"
  "3cec44ea1fa80766a4b7938e744ae9a076bdee61e583d95263"
  "2db5e879900a1b500c61518049defdbb32563d32249cbfc948"
  "7c5c721e8e807e1416e6bcdaef848b7a2309169f484f93b6f0"
  "fa84d68f1e75ed4d20baa664029b22257428c179b5f76802d8"
  "a50a5a3b813b057f765812fdc9085b6d59ba9d6f6521ee1278"
  "df96658c2b5ba52d116a1b3d57a18613a003ff323ece8f3c84"
  "511b7a56d95075a6e7fd3b78114e02a86ab578ba29c7f3aef9"
  "8aeeaf3e8ff8e94bbb7bf7eadbe158ab2f3431ce87d8c0345d"
  "d30522a58bbcef704de5b09eedec1c8f69b401c526d242b01e"
  "7485c56e91499bc7b83953d5b79862ec1d37bb5814189c4d69"
  "a2829125cfa8379fee985337e7d13de073a29c256ef982f1ba"
  "b0c776613e01ee96bde60fe6c73b5850ef54600f3e9a2a0a5b"
  "a6f1f3b9b2be211dde87294b1c4170cc51f5b2ae373ef082ba"
  "d96ac5efdf1d2afd213be345b830b972b62044a12bcabeab2a"
  "934406686be38602c93e0ac8a764dcfb756794ad160d2c35af"
  "a5986cc2975e96031cffe0bfecadc0076c7d80e0e52b68c23a"
  "34b12a2f71cbfec05b881d3fac344c5dc7f7a36ad9c3be38bf"
  "c3cf3e1b7001961e8b65a209deadecf310f8060caaf4d994c5"
  "936a69588ce6d0adde2306a10387d066f000ad7502b93c193c"
  "67b1d2a0c2994e4cc25c76fda207e85676bd57828ae146b271"
  "9ff88991822b717ad5914d64abbbcff1378cac976c612c1047"
  "5cdf826b2a58d1bcd0b9dbe9724c270794a6faf59974665a23"
  "4176ed331f160cdaac244c396377117af50c79b13261707d4f"
  "f5efdf62dd6811e5ceca39cd6c1da3cb474a3b1b6c6b72d1c6"
  "4a6be2e74b0bc3aa32c8c9aa10eac1a048f283a8427821a852"
  "2d25fc5c068d74242653ce821c7bce20b3a8208a8a0022a08a"
  "0432a49a0c120c9a8012198f910c128fb92498bfa934a83f81"
  "84b91281980b980112090a1102998a992a992a882e8c2f048d"
  "06ad8c0726ad8c8f2605a4ab0a00a0aa082028202820282028"
  "00000000000001010101018b218b218b218b21832983288228"
  "822808080808080808080000000000001000000008aaaaaaaa"
  "aaaaaaaa2aaaaabaaaaaaaa200000000000000008";

#elif DSFMT_MEXP == 19937

int km_dsfmt_period = 19937;

std::string km_dsfmt_poly = 
  "10455110544511444054554514541dbbb0a20820a288a00a80"
  "a82208280b73237d2ff2ab5e2fcac914510101df18802aaaa0"
  "8a022820a02aaa202ae822c364b91fb08598fe4410154041f0"
  "28a266586e59cf12ce316b3a56af45cb49d02c7059cb89d2d8"
  "1b0d288badbb25fa4e53b280aa88a22aa82abe7fc0645b7d7a"
  "4650c1dec48f21224e3d0e6e04c062c2273ef0d8361415c724"
  "dbc8f79118d5fac429f35dc460f6007e54c3966c28808e4c93"
  "08cc46e2e0e153bd922223796d4101af867e16e6641e6831e0"
  "6ebbd2beaf52b2b47076dbf3a3e36c6d14d19dbf5d4b2b44b4"
  "d3aa6e1ea102578d765f08e1cb0749220075b8aae81c6e9969"
  "2a56b35ddd4cf91b1034f1398c98e2d4ac8dbed09bc73ede09"
  "8514cf3ffdf45cbb59335e3ec42f5f6a5672acc4ca8daa02a2"
  "502350ac0485f8b36f27d7a1d4d4b22fc7601e22a4f7c6ba53"
  "782b335837a21a068e8fcf3fdebb28860157301519cdea328b"
  "1ef4b8d5bc512ce65ff33298c34cc98ea1558f7b6d4db594f4"
  "bcab58d9f7bcf5cc59e259216de13f77569bbcee1c8abd55de"
  "985b7129e611d156c08cafa022ad7a2206a34a61e5c4c91e75"
  "495112003ec03c182a0155d9221382d0b30f45a00d6c19740f"
  "9ecebb448de78dfc0342f14166f54afdb97d00ac1a390355ce"
  "2aa9de1b3c919d8dd188fc9451ce9c86fa560a2da9dcaa681e"
  "fd21fe5b6055f8e35a833e5704936245d06e387bf9956251bf"
  "0a252628be7a3cb0edab4aaaf141e6d7a9929cc03afa76ca43"
  "3016d880def9e4cf10f7274e58aa35c372b7b7fb210fe0dc1a"
  "6b8445e7774ad6001b9aa1f2a01a790ee849e7ac810e0a646e"
  "733a7121bd727a9b593e0c9f5306dff5105af5967e3cee8106"
  "9100d7e91a9c266e64c9e073a6e527765308879b22ca062668"
  "f9a564da6314dcad51405f160e8259582b7c06c313c84b0acb"
  "a44958c052e6be540a7688e240232bee40b990dc48ee07d560"
  "786ca34e7df1bbdd2bca38a30c548ec57e91906b8417ff0da6"
  "8db9c154d8ad83b06a6fc2b2e14ca5fbe7bd50382949c9b1f6"
  "e8540d9d43b35fa76a6ded27c2f17095a4f330626c5e86e8ff"
  "88ae53e05a434356a04a1ddae43b6e2ab883719360fbece72b"
  "6090ab17414ca7006e49d183813422c808fde53a30f872254b"
  "b554653aef86855f95a9361782100de2402efd749cc8cf6a83"
  "7066c1c40c0160e415d8119e53a09877f1160ef8b99b2839e9"
  "b8c09b1e461e906041344c8fc2ef0a8eda04e319da41e001e6"
  "0bc64dcdfc0593dff0f4b390580e1cd5b3c16204e77df10217"
  "791f99de49fcdc9160b793fd980bff7ae0cbd570425eb43935"
  "2e5032e03797461376b5fb92aa156ad64935cc201a162f10f0"
  "4b6d2d20a0415ae16f299e98baa86466f6f517f05f43025488"
  "4a4010ac196540b0644e3c274323d4f0206780d38175f1e41f"
  "cb65bd387be073abee61b99d43f6b9106953ae4f6906e6ac07"
  "41e26af05fa9150c4f380558668aa667e404e3784b839d631e"
  "35af015024dbfc3dce4685574cd1e58eb72c70011090a2a876"
  "b65e34cb6cf297d24cb61ccca5a9706f34ae1f66345998f850"
  "de4e48d77cdf6e00fb0d2210ec9fb53fed02a781f7dfcdb609"
  "b9f955504f450e4b7b623cb0f5ba1ea09d92cd8d14f7e827b4"
  "580855f3a7dc2e5a45817df9e26adb5934f6026f745cb0f65e"
  "71c590bd954d1abc3826379719b7c6f4a0cbe6eb22a903b98b"
  "ef785bb96efe2fc96988722c91f3e59d28d8244c8bfb59ee26"
  "082d82cad937ee70f178b44082533308ca24f236d8b91ca7af"
  "5b3d865c90d61410e1ffe39be6433a12ef2932e101b4bb34be"
  "fa76748e0364a96f06e7932f44297fd5f85765b662c3ade19d"
  "9a9d9479f6de20b6b753d3dcbadf63e344578b98af85b4c4c6"
  "3be9d154864f5d341f210f3503a60efec38ee59069499d0049"
  "aedfaef9264a7ce9de460a01e5437254fc68dfaebaa5e0e791"
  "380806bc149aedbc1d771457770937a5d606fc5ae728993783"
  "e6c45e1f5e1b9492a32f9df46a01020792a3803af04837a390"
  "5e7cc6ae68c512cb58f4facb457476729bac1ac89a7a32dc68"
  "57edbd6624ebeffed9d4e84a2f4ded9759962635aac94ca72d"
  "039c14af6e932fc84c25de28688acab0f41ae931a0f35b9c41"
  "95483d902f95e0d3e745e08f334cf5062da9fbd18fbb9efeb2"
  "75a50a8a479939aa3e376821a030f0d241a4c4f6e3298a43a7"
  "f2166db483946c5ca623087b6252e27b09805d92a834ad914b"
  "9555f9c36150bb17d7e85f6b1a10b41a5c36a1fd0fdc354ec9"
  "1720b245b0abcde6b38fdd33f471aa6ba1dbb4f178c3c6d814"
  "0a1f0700e072b0f9aafe1208abc94eea750b84e74ba6781b48"
  "c398ddfd2842e5447a11767e4f467864e4447382379010669f"
  "07870339ca42ac290063b40eaba1daa63cf6cd70c5a600c018"
  "f6933dc8bec000a521344cad7320ba121dd21f4723d38a8d3a"
  "b8c1f0f6c5a6995b796af79407ae1a7ce6d78922677d9990dd"
  "2a588f8a3323d80e9a69417f5224565afa60b64604e7316229"
  "514dcb7282b4e638981a5751d114da1ac9bf31f0e2fff5213f"
  "2020f9f2f31a8fd0c614e80c1a056d4b1af3ded2f696578f56"
  "427962ca54f4a28a502a0ac59013416abd81515fb1956ccb49"
  "4c05ef61cab48474b6b1609cc5a3871a5111f8bf0a76b378f0"
  "e646687829e30f5762156da66c1b1c7abc0eb84e6ff2b9f5b2"
  "2d34540ab97d643e8dd2e35f6e9e4fc2c30d8af88b2caab7bd"
  "5d4a6cf967e8ef79967c1ae85bf7d410a79f4630f13fdc6507"
  "d339909b81a29d84741103371310e5b4e279758431df627553"
  "b6826fc4c98e5fb6551315b0bd811b7b0f357198210dd99ccd"
  "8fba2904114c3e0b344eba43832b3c507e8b6b586e4ab3dc7a"
  "2ec71e150c54a13eca2340328d0b3e419ab2ba862ee93fc239"
  "fb46d907628055e105318e7fa52f9a83deb0e3cb02c62b8817"
  "702ead02a315f76aa1d08860cc5214a867808e33f8e0752419"
  "56f148f876f3bc66566773610c9c5935b559c0ac47d84b6bfb"
  "112f59842be58df51055cf9180264f53f7795d4c934718bc65"
  "f359e34a8d230408854685b59c3a9f4d73a229bb465d4da316"
  "5404c6786c767299f57dfa85a83492fb4f61386441c928224c"
  "d88a7f4b36f245b7aa2b5c97b545ac4db8afe9a1a87e27b57d"
  "94c2bbffdb6e88f812aa27e0908048812086c2a72289d7bf13"
  "6b3a1042a44d1913d39caec24ffda22814706f080b6cbe00e9"
  "cd442ccdcb600a436c0daeacbc5482021ba8a06c1fedbb3337"
  "93557d5175b9313799ff91dfa620380a9e2a10132f0818bba7"
  "2072e359726e2bd1f2ec98e0face32e0f88ee2c6f7cef7c2fb"
  "ceffe8d3ccdff97b7ff71d861ba8b98237ccfb00176ee02206"
  "ccc08026cee082a88a8a349a1c9016983ea10789272105032f"
  "89b3113fd9b75b35c884622ec884622ee8aee2aaa2aeae8686"
  "8c868ea68ea6862e0624062ea22aa66ee66ee44ee64ce64ce6"
  "4ce646464c4808081808080a0a0828282828282822222a8898"
  "888888888aaaa2aaaaaaaaaaaaa2000000000000000008";

#endif