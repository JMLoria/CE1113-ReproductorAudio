transcript on
if ![file isdirectory Prueba-Audio_iputf_libs] {
	file mkdir Prueba-Audio_iputf_libs
}

if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

###### Libraries for IPUTF cores 
###### End libraries for IPUTF cores 
###### MIF file copy and HDL compilation commands for IPUTF cores 


vlog "C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Prueba-Audio/pll_audio_sim/pll_audio.vo"

vlog -vlog01compat -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Prueba-Audio {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Prueba-Audio/pll_audio.vo}
vlib pll_audio
vmap pll_audio pll_audio
vlog -vlog01compat -work pll_audio +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Prueba-Audio/pll_audio {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Prueba-Audio/pll_audio/pll_audio_0002.v}
vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros/tb_AudioFilter.sv}
vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros/ByPassFilter.sv}
vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Filtros/BassBoostFilter.sv}
vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Prueba-Audio {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Prueba-Audio/top_audio_test.sv}
vlog -sv -work work +incdir+C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Prueba-Audio {C:/intelFPGA_lite/P2-Empotrados/CE1113-ReproductorAudio/hw/Prueba-Audio/ToneGenerator.sv}

