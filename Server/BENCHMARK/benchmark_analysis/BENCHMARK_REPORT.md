# PBVR ベンチマークレポート

このレポートは既存のPBVRタイマーCSV/ログから生成しています。並列比較では平均だけでなくMPI max時間を使います。

## スケーリング概要

| case | nodes | mpi | omp | total_cores | total_time | speedup | efficiency |
| --- | --- | --- | --- | --- | --- | --- | --- |
| ens_Hydrogen_unstruct_mpi4 | 1 | 1 | 1 | 1 | 13.909319 | 1.0 | 1.0 |

## ボトルネック候補

| priority | section | symptom | evidence | hypothesis | next_check |
| --- | --- | --- | --- | --- | --- |
| 1 | async_io_wait | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=4.19483, contribution=0.302, max/avg=20.422 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | async_io_wait | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=3.39086, contribution=0.244, max/avg=14.628 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | async_io_wait | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=3.9137, contribution=0.281, max/avg=13.982 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | async_io_wait | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=5.074, contribution=0.365, max/avg=18.695 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | ensemble_generate_particles_total | 全体時間への寄与が大きい | case=ens_Hydrogen_unstruct_mpi4, opt_max=13.9093, contribution=1.000, max/avg=1.000 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | ensemble_generate_particles_total | 全体時間への寄与が大きい | case=ens_Hydrogen_unstruct_mpi4, opt_max=12.7319, contribution=0.915, max/avg=1.000 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | ensemble_generate_particles_total | 全体時間への寄与が大きい; 基準ケースより遅い | case=ens_Hydrogen_unstruct_mpi4, opt_max=15.087, contribution=1.085, max/avg=1.000 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | ensemble_generate_particles_total | 全体時間への寄与が大きい | case=ens_Hydrogen_unstruct_mpi4, opt_max=15.067, contribution=1.083, max/avg=1.000 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | final_barrier_state | 全体時間への寄与が大きい; 要確認; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=10.3565, contribution=0.745, max/avg=1.221 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | final_barrier_state | 全体時間への寄与が大きい; 要確認; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=8.66834, contribution=0.623, max/avg=1.295 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | final_barrier_state | 全体時間への寄与が大きい; 基準ケースより遅い; 要確認; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=10.8865, contribution=0.783, max/avg=1.216 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | final_barrier_state | 全体時間への寄与が大きい; 要確認; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=10.5776, contribution=0.760, max/avg=1.263 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | mpi_shift_size_exchange | 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=0.013469, contribution=0.001, max/avg=2.710 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | mpi_shift_size_exchange | 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=0.018457, contribution=0.001, max/avg=3.182 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | mpi_shift_size_exchange | 基準ケースより遅い; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=0.138034, contribution=0.010, max/avg=9.810 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | mpi_shift_size_exchange | 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=0.021033, contribution=0.002, max/avg=3.217 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_coord_minmax | 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=0.000985, contribution=0.000, max/avg=30.425 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_coord_minmax | 基準ケースより遅い; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=0.001356, contribution=0.000, max/avg=34.353 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_coord_minmax | 基準ケースより遅い; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=0.001318, contribution=0.000, max/avg=34.508 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_coord_minmax | 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=0.001315, contribution=0.000, max/avg=34.404 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_history | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=6.15908, contribution=0.443, max/avg=11.166 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_history | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=6.49847, contribution=0.467, max/avg=13.649 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_history | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=7.58456, contribution=0.545, max/avg=15.665 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_history | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=8.19088, contribution=0.589, max/avg=15.139 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_particles_ave | 基準ケースより遅い; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=0.009601, contribution=0.001, max/avg=2.509 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_particles_cov | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=3.49787, contribution=0.251, max/avg=3.959 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_particles_cov | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=3.73571, contribution=0.269, max/avg=3.777 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_particles_cov | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=3.65224, contribution=0.263, max/avg=3.862 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_particles_cov | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=4.46478, contribution=0.321, max/avg=4.064 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_particles_var | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=3.4977, contribution=0.251, max/avg=14.412 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_particles_var | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=3.73552, contribution=0.269, max/avg=14.368 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_particles_var | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=3.32578, contribution=0.239, max/avg=14.549 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | output_particles_var | 全体時間への寄与が大きい; 強い負荷不均衡または待ち; 通信/同期/I/O候補 | case=ens_Hydrogen_unstruct_mpi4, opt_max=4.4646, contribution=0.321, max/avg=14.897 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | sampling_prepare | 強い負荷不均衡または待ち | case=ens_Hydrogen_unstruct_mpi4, opt_max=1.8e-05, contribution=0.000, max/avg=6.000 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | sampling_prepare | 強い負荷不均衡または待ち | case=ens_Hydrogen_unstruct_mpi4, opt_max=1.9e-05, contribution=0.000, max/avg=5.897 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | sampling_prepare | 強い負荷不均衡または待ち | case=ens_Hydrogen_unstruct_mpi4, opt_max=2.4e-05, contribution=0.000, max/avg=7.082 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | set_parameter_path | 全体時間への寄与が大きい; 強い負荷不均衡または待ち | case=ens_Hydrogen_unstruct_mpi4, opt_max=2.40131, contribution=0.173, max/avg=5.336 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | set_parameter_path | 全体時間への寄与が大きい; 強い負荷不均衡または待ち | case=ens_Hydrogen_unstruct_mpi4, opt_max=2.94499, contribution=0.212, max/avg=8.462 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | set_parameter_path | 全体時間への寄与が大きい; 強い負荷不均衡または待ち | case=ens_Hydrogen_unstruct_mpi4, opt_max=2.90438, contribution=0.209, max/avg=6.459 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |
| 1 | set_parameter_path | 全体時間への寄与が大きい; 強い負荷不均衡または待ち | case=ens_Hydrogen_unstruct_mpi4, opt_max=3.37182, contribution=0.242, max/avg=6.641 | 負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト | rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する |

先頭 40 / 115 行を表示。


## 区間別概要

| case | section | serial_time | opt_avg | opt_max | speedup_vs_serial | max_over_avg | contribution |
| --- | --- | --- | --- | --- | --- | --- | --- |
| ens_Hydrogen_unstruct_mpi4 | ensemble_generate_particles_total | 15.066991 | 13.9059922 | 13.909319 | 1.0832299553989666 | 1.0002392349968383 | 1.0 |
| ens_Hydrogen_unstruct_mpi4 | set_parameter_path | 3.371816 | 0.450011031 | 2.401314 | 1.4041545587124382 | 5.336122527183117 | 0.17264065911494303 |
| ens_Hydrogen_unstruct_mpi4 | read_parameter_file | 3.374415 | 1.95220984 | 2.403886 | 1.4037333717156304 | 1.2313666034999597 | 0.17282557111530766 |
| ens_Hydrogen_unstruct_mpi4 | init_transfer_functions | 0.000535 | 0.00049934375 | 0.000513 | 1.0428849902534112 | 1.0273483947681332 | 3.68817481287186e-05 |
| ens_Hydrogen_unstruct_mpi4 | create_cells | 0.000595 | 0.00051821875 | 0.000592 | 1.0050675675675678 | 1.1423747210999216 | 4.2561393552049525e-05 |
| ens_Hydrogen_unstruct_mpi4 | sampling_prepare | 2.4e-05 | 3e-06 | 1.8e-05 | 1.3333333333333333 | 6.0 | 1.2940964255690735e-06 |
| ens_Hydrogen_unstruct_mpi4 | omp_uniform_sampling | 0.333173 | 0.361640875 | 0.365342 | 0.9119482567019396 | 1.010234255184788 | 0.026265987572792027 |
| ens_Hydrogen_unstruct_mpi4 | uniform_thread_setup | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | uniform_cell_index_setup | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | uniform_bind_cell_array | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | uniform_volume_calculation | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | uniform_particle_count_calculation | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | uniform_particle_sampling_loop | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | uniform_local_coord_generation | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | uniform_flush_prepare | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | uniform_calculate_scalars_array | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | uniform_store_particle_data | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | thread_particle_merge | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | mpi_shift_exchange | 0.727008 | 0.720191406 | 0.728668 | 0.9977218705912707 | 1.0117699182875282 | 0.05238703634592031 |
| ens_Hydrogen_unstruct_mpi4 | mpi_shift_size_exchange | 0.021033 | 0.00496990625 | 0.013469 | 1.561585863835474 | 2.710111483491263 | 0.0009683435975549917 |
| ens_Hydrogen_unstruct_mpi4 | mpi_shift_alloc_recv_buffer | 0.122485 | 0.118224156 | 0.124007 | 0.9877264993105227 | 1.0489142337374775 | 0.008915389746974672 |
| ens_Hydrogen_unstruct_mpi4 | mpi_shift_payload_all | 0.160203 | 0.160230875 | 0.168154 | 0.9527159627484331 | 1.0494481790728536 | 0.012089305019174554 |
| ens_Hydrogen_unstruct_mpi4 | omp_shift_interpolation | 0.451962 | 0.436565812 | 0.439027 | 1.0294628804150996 | 1.0056376104870073 | 0.031563515079350755 |
| ens_Hydrogen_unstruct_mpi4 | shift_calculate_scalars_array | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | stat_average_variance | 0.013775 | 0.0148646563 | 0.015504 | 0.8884803921568628 | 1.0430109978392168 | 0.001114648387890162 |
| ens_Hydrogen_unstruct_mpi4 | stat_histogram | 0.008866 | 0.009001375 | 0.00913 | 0.9710843373493977 | 1.014289483551124 | 0.00065639446474698 |
| ens_Hydrogen_unstruct_mpi4 | omp_rejection | 0.022882 | 0.0252829062 | 0.025805 | 0.8867273784150358 | 1.0206500706789792 | 0.0018552310145449969 |
| ens_Hydrogen_unstruct_mpi4 | rejection_thread_merge | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| ens_Hydrogen_unstruct_mpi4 | cleanup_tfs | 0.000172 | 0.00015621875 | 0.000171 | 1.0058479532163742 | 1.094618923784757 | 1.22939160429062e-05 |
| ens_Hydrogen_unstruct_mpi4 | output_coord_minmax | 0.001315 | 3.2375e-05 | 0.000985 | 1.3350253807106598 | 30.424710424710423 | 7.08158321769743e-05 |
| ens_Hydrogen_unstruct_mpi4 | output_particles_ave | 0.009464 | 0.003827125 | 0.009601 | 0.9857306530569733 | 2.508671653003234 | 0.0006902566545493709 |
| ens_Hydrogen_unstruct_mpi4 | output_particles_var | 4.464595 | 0.242700219 | 3.497697 | 1.2764384679404763 | 14.411593917844797 | 0.2514642880790929 |
| ens_Hydrogen_unstruct_mpi4 | output_particles_cov | 4.464777 | 0.883550406 | 3.497869 | 1.2764277335714973 | 3.9588788327714264 | 0.2514766538893817 |
| ens_Hydrogen_unstruct_mpi4 | output_history | 8.190875 | 0.551572875 | 6.159084 | 1.329885255664641 | 11.166401175909893 | 0.4428026993988706 |
| ens_Hydrogen_unstruct_mpi4 | cleanup_cells | 0.000125 | 9.765625e-05 | 0.00012 | 1.0416666666666667 | 1.2288 | 8.627309503793824e-06 |
| ens_Hydrogen_unstruct_mpi4 | write_tf_file | 0.005484 | 0.000148375 | 0.004748 | 1.1550126368997473 | 32.0 | 0.0003413538793667756 |
| ens_Hydrogen_unstruct_mpi4 | async_io_wait | 5.074002 | 0.205408781 | 4.194826 | 1.2095858088035119 | 20.42184360171048 | 0.30158385180467856 |
| ens_Hydrogen_unstruct_mpi4 | final_barrier_state | 10.577645 | 8.47890366 | 10.356526 | 1.0213506923074398 | 1.2214463585496147 | 0.7445746265507319 |
| ens_Hydrogen_unstruct_mpi4 | ensemble_generate_particles_total | 15.066991 | 12.7294041 | 12.731918 | 1.1834030819237134 | 1.0001974876420177 | 0.9153516430243638 |
| ens_Hydrogen_unstruct_mpi4 | set_parameter_path | 3.371816 | 0.348032528 | 2.944986 | 1.1449344750705097 | 8.461812511961526 | 0.21172754755283132 |

先頭 40 / 152 行を表示。


## タイマー概要

| case | section | min | avg | max | max_over_avg | thread_avg_sec | thread_max_sec |
| --- | --- | --- | --- | --- | --- | --- | --- |
| ens_Hydrogen_unstruct_mpi4 | ensemble_generate_particles_total | 13.885589 | 13.9059922 | 13.909319 | 1.0002392349968383 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | set_parameter_path | 0.000511 | 0.450011031 | 2.401314 | 5.336122527183117 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | read_parameter_file | 0.001688 | 1.95220984 | 2.403886 | 1.2313666034999597 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | init_transfer_functions | 0.000492 | 0.00049934375 | 0.000513 | 1.0273483947681332 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | create_cells | 0.000132 | 0.00051821875 | 0.000592 | 1.1423747210999216 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | sampling_prepare | 1e-06 | 3e-06 | 1.8e-05 | 6.0 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | omp_uniform_sampling | 0.353919 | 0.361640875 | 0.365342 | 1.010234255184788 | 0.218913648 | 0.363179 |
| ens_Hydrogen_unstruct_mpi4 | uniform_thread_setup | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.000143153646 | 0.000541 |
| ens_Hydrogen_unstruct_mpi4 | uniform_cell_index_setup | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.000102046875 | 0.000136 |
| ens_Hydrogen_unstruct_mpi4 | uniform_bind_cell_array | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.0156110677 | 0.016422 |
| ens_Hydrogen_unstruct_mpi4 | uniform_volume_calculation | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.0169752188 | 0.022702 |
| ens_Hydrogen_unstruct_mpi4 | uniform_particle_count_calculation | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.00315259635 | 0.003309 |
| ens_Hydrogen_unstruct_mpi4 | uniform_particle_sampling_loop | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.0909027318 | 0.092789 |
| ens_Hydrogen_unstruct_mpi4 | uniform_local_coord_generation | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.00620068229 | 0.007023 |
| ens_Hydrogen_unstruct_mpi4 | uniform_flush_prepare | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.00915619531 | 0.009992 |
| ens_Hydrogen_unstruct_mpi4 | uniform_calculate_scalars_array | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.0514207161 | 0.053494 |
| ens_Hydrogen_unstruct_mpi4 | uniform_store_particle_data | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.0175778906 | 0.019582 |
| ens_Hydrogen_unstruct_mpi4 | thread_particle_merge | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.0911223359 | 0.235388 |
| ens_Hydrogen_unstruct_mpi4 | mpi_shift_exchange | 0.712806 | 0.720191406 | 0.728668 | 1.0117699182875282 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | mpi_shift_size_exchange | 0.000136 | 0.00496990625 | 0.013469 | 2.710111483491263 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | mpi_shift_alloc_recv_buffer | 0.111987 | 0.118224156 | 0.124007 | 1.0489142337374775 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | mpi_shift_payload_all | 0.150864 | 0.160230875 | 0.168154 | 1.0494481790728536 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | omp_shift_interpolation | 0.434442 | 0.436565812 | 0.439027 | 1.0056376104870073 | 0.436398281 | 0.438915 |
| ens_Hydrogen_unstruct_mpi4 | shift_calculate_scalars_array | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.352847417 | 0.356666 |
| ens_Hydrogen_unstruct_mpi4 | stat_average_variance | 0.014393 | 0.0148646563 | 0.015504 | 1.0430109978392168 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | stat_histogram | 0.008881 | 0.009001375 | 0.00913 | 1.014289483551124 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | omp_rejection | 0.025027 | 0.0252829062 | 0.025805 | 1.0206500706789792 | 0.0248279453 | 0.025793 |
| ens_Hydrogen_unstruct_mpi4 | rejection_thread_merge | 1.79769313e+308 | 0.0 | 0.0 | 0.0 | 0.0004935625 | 0.001142 |
| ens_Hydrogen_unstruct_mpi4 | cleanup_tfs | 0.000145 | 0.00015621875 | 0.000171 | 1.094618923784757 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | output_coord_minmax | 1e-06 | 3.2375e-05 | 0.000985 | 30.424710424710423 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | output_particles_ave | 0.001586 | 0.003827125 | 0.009601 | 2.508671653003234 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | output_particles_var | 0.001224 | 0.242700219 | 3.497697 | 14.411593917844797 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | output_particles_cov | 0.004546 | 0.883550406 | 3.497869 | 3.9588788327714264 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | output_history | 3.4e-05 | 0.551572875 | 6.159084 | 11.166401175909893 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | cleanup_cells | 8.8e-05 | 9.765625e-05 | 0.00012 | 1.2288 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | write_tf_file | 0.004748 | 0.000148375 | 0.004748 | 32.0 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | async_io_wait | 1.1e-05 | 0.205408781 | 4.194826 | 20.42184360171048 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | final_barrier_state | 2.8e-05 | 8.47890366 | 10.356526 | 1.2214463585496147 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | ensemble_generate_particles_total | 12.706902 | 12.7294041 | 12.731918 | 1.0001974876420177 | 0 | 0 |
| ens_Hydrogen_unstruct_mpi4 | set_parameter_path | 0.000442 | 0.348032528 | 2.944986 | 8.461812511961526 | 0 | 0 |

先頭 40 / 152 行を表示。

