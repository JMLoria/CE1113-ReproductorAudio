transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros/LowPassFilter.sv}
vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros/HighPassFilter.sv}
vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros/ByPassFilter.sv}
vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros/BassBoostFilter.sv}
vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros/AudioFilter.sv}

vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros/tb_AudioFilter.sv}

vsim -t 1ps -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cyclonev_ver -L cyclonev_hssi_ver -L cyclonev_pcie_hip_ver -L rtl_work -L work -voptargs="+acc"  tb_AudioFilter

add wave *
view structure
view signals
run -all
