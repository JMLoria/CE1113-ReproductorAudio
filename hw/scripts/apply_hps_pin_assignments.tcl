# apply_hps_pin_assignments.tcl
#
# Ejecuta el script generado por Platform Designer que añade las
# asignaciones específicas del HPS DDR3 (IO standards, OCT calibration,
# drive strengths, etc.) al .qsf.
#
# Uso: en la TCL Console de Quartus:
#   source scripts/apply_hps_pin_assignments.tcl
#
# Este script debe ejecutarse:
# - La primera vez que se clona el repo
# - Después de regenerar el sistema en Platform Designer

source soc_system/synthesis/submodules/hps_sdram_p0_pin_assignments.tcl

puts "==========================================="
puts "HPS DDR3 pin assignments applied successfully"
puts "Now you can run: Processing -> Start Compilation"
puts "==========================================="