PBVR_DIR = ../../../..

include $(PBVR_DIR)/pbvr.conf
include $(PBVR_DIR)/arch/$(PBVR_MACHINE)

MPICXX ?= mpic++
MPICXXFLAGS ?= $(CXXFLAGS)
MPILD ?= $(MPICXX)
MPILDFLAGS ?= $(LDFLAGS)

export MPICXX MPICXXFLAGS MPILD MPILDFLAGS

.PHONY: all clean

# アンサンブルPBVR 用のサンプル。通常サンプル(親ディレクトリの Hydrogen_*)とは
# 別に置く。KVS のインストール先は各サンプルの PBVR_KVS_INSTALL_DIR で指定する。
# 必要に応じて環境変数 PBVR_KVS_INSTALL_DIR で上書きできる。

# --- 非構造格子 ---
SUBDIRS += ens_Hydrogen_unstruct
SUBDIRS += ens_Hydrogen_unstruct_tetra
SUBDIRS += ens_Hydrogen_unstruct_prism
SUBDIRS += ens_Hydrogen_unstruct_pyramid
SUBDIRS += ens_Hydrogen_unstruct_quadtetra
SUBDIRS += ens_Hydrogen_unstruct_quadhexa

# --- 構造格子 ---
SUBDIRS += ens_Hydrogen_struct
SUBDIRS += ens_Hydrogen_struct_256
SUBDIRS += ens_Hydrogen_struct_sine

# --- スケーリング計測用(MPI数・分割を固定したもの) ---
SUBDIRS += ens_Hydrogen_unstruct_strong
SUBDIRS += ens_Hydrogen_unstruct_4eweak_scale
SUBDIRS += ens_Hydrogen_unstruct_mpi4
# mpi2 / mpi8 / mpi16 は main.cpp が素のグローバル mpi_per_ens を参照しており
# 現在のコードではビルドできない(移動前からの既存バグ。ens_Hydrogen_unstruct_strong
# では hydro.mpi_per_ens に修正済み)。直すまで一括ビルドから外す。
# SUBDIRS += ens_Hydrogen_unstruct_mpi2
# SUBDIRS += ens_Hydrogen_unstruct_mpi8
# SUBDIRS += ens_Hydrogen_unstruct_mpi16
SUBDIRS += ens_Hydrogen_unstruct_weak_zslab4

# --- 検証用 ---
SUBDIRS += ens_Hydrogen_unstruct_analytic
SUBDIRS += ens_Hydrogen_unstruct_sine

# SUBDIRS には git 管理外のサンプル(.gitignore 登録済み)も挙げてある。
# クローンした環境には実体が無いので、無いものは飛ばす。
all:
	for i in $(SUBDIRS); do \
	if [ -d $$i ]; then $(MAKE) -C $$i; \
	else echo "skip $$i (このリポジトリには含まれていません)"; fi; done

clean:
	for i in $(SUBDIRS); do \
	if [ -d $$i ]; then $(MAKE) -C $$i clean; fi; done
