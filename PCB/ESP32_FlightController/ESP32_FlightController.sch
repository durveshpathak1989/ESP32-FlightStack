(kicad_sch
	(version 20260306)
	(generator "eeschema")
	(generator_version "10.0")
	(uuid "ee2f03c1-c37f-476e-8725-9667899dc27d")
	(paper "A3")
	(title_block
		(title "ESP32 FlightStack Integrated Flight Controller")
		(date "2026-07-20")
		(rev "1.0")
		(company "durveshpathak1989 / ESP32-FlightStack")
		(comment 1 "ESP32-WROOM-32E + MPU-9250 + BMP280 + GPS + iBUS + 4 PWM ESC")
		(comment 2 "Pin mapping matches RC_FlightController.ino")
		(comment 3 "Onboard 3S-to-5V buck and 3.3V LDO")
		(comment 4 "Prototype flight controller — verify on bench before flight")
	)
	(lib_symbols
		(symbol "Connector_Generic:Conn_01x02"
			(pin_names
				(offset 1.016)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "J"
				(at 0 2.54 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "Conn_01x02"
				(at 0 -5.08 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Generic connector, single row, 01x02, script generated (kicad-library-utils/schlib/autogen/connector/)"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "connector"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "Connector*:*_1x??_*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "Conn_01x02_1_1"
				(rectangle
					(start -1.27 1.27)
					(end 1.27 -3.81)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
				(rectangle
					(start -1.27 0.127)
					(end 0 -0.127)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 -2.413)
					(end 0 -2.667)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(pin passive line
					(at -5.08 0 0)
					(length 3.81)
					(name "Pin_1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 -2.54 0)
					(length 3.81)
					(name "Pin_2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Connector_Generic:Conn_01x03"
			(pin_names
				(offset 1.016)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "J"
				(at 0 5.08 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "Conn_01x03"
				(at 0 -5.08 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Generic connector, single row, 01x03, script generated (kicad-library-utils/schlib/autogen/connector/)"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "connector"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "Connector*:*_1x??_*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "Conn_01x03_1_1"
				(rectangle
					(start -1.27 3.81)
					(end 1.27 -3.81)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
				(rectangle
					(start -1.27 2.667)
					(end 0 2.413)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 0.127)
					(end 0 -0.127)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 -2.413)
					(end 0 -2.667)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(pin passive line
					(at -5.08 2.54 0)
					(length 3.81)
					(name "Pin_1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 0 0)
					(length 3.81)
					(name "Pin_2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 -2.54 0)
					(length 3.81)
					(name "Pin_3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Connector_Generic:Conn_01x04"
			(pin_names
				(offset 1.016)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "J"
				(at 0 5.08 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "Conn_01x04"
				(at 0 -7.62 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Generic connector, single row, 01x04, script generated (kicad-library-utils/schlib/autogen/connector/)"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "connector"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "Connector*:*_1x??_*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "Conn_01x04_1_1"
				(rectangle
					(start -1.27 3.81)
					(end 1.27 -6.35)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
				(rectangle
					(start -1.27 2.667)
					(end 0 2.413)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 0.127)
					(end 0 -0.127)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 -2.413)
					(end 0 -2.667)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 -4.953)
					(end 0 -5.207)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(pin passive line
					(at -5.08 2.54 0)
					(length 3.81)
					(name "Pin_1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 0 0)
					(length 3.81)
					(name "Pin_2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 -2.54 0)
					(length 3.81)
					(name "Pin_3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 -5.08 0)
					(length 3.81)
					(name "Pin_4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Connector_Generic:Conn_01x06"
			(pin_names
				(offset 1.016)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "J"
				(at 0 7.62 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "Conn_01x06"
				(at 0 -10.16 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Generic connector, single row, 01x06, script generated (kicad-library-utils/schlib/autogen/connector/)"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "connector"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "Connector*:*_1x??_*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "Conn_01x06_1_1"
				(rectangle
					(start -1.27 6.35)
					(end 1.27 -8.89)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
				(rectangle
					(start -1.27 5.207)
					(end 0 4.953)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 2.667)
					(end 0 2.413)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 0.127)
					(end 0 -0.127)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 -2.413)
					(end 0 -2.667)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 -4.953)
					(end 0 -5.207)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start -1.27 -7.493)
					(end 0 -7.747)
					(stroke
						(width 0.1524)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(pin passive line
					(at -5.08 5.08 0)
					(length 3.81)
					(name "Pin_1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 2.54 0)
					(length 3.81)
					(name "Pin_2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 0 0)
					(length 3.81)
					(name "Pin_3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 -2.54 0)
					(length 3.81)
					(name "Pin_4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 -5.08 0)
					(length 3.81)
					(name "Pin_5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at -5.08 -7.62 0)
					(length 3.81)
					(name "Pin_6"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "6"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Device:C"
			(pin_numbers
				(hide yes)
			)
			(pin_names
				(offset 0.254)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "C"
				(at 0.635 2.54 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Value" "C"
				(at 0.635 -2.54 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Footprint" ""
				(at 0.9652 -3.81 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Unpolarized capacitor"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "cap capacitor"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "C_*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "C_0_1"
				(polyline
					(pts
						(xy -2.032 0.762) (xy 2.032 0.762)
					)
					(stroke
						(width 0.508)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy -2.032 -0.762) (xy 2.032 -0.762)
					)
					(stroke
						(width 0.508)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "C_1_1"
				(pin passive line
					(at 0 3.81 270)
					(length 2.794)
					(name ""
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 0 -3.81 90)
					(length 2.794)
					(name ""
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Device:C_Polarized"
			(pin_numbers
				(hide yes)
			)
			(pin_names
				(offset 0.254)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "C"
				(at 0.635 2.54 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Value" "C_Polarized"
				(at 0.635 -2.54 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Footprint" ""
				(at 0.9652 -3.81 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Polarized capacitor"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "cap capacitor"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "CP_*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "C_Polarized_0_1"
				(rectangle
					(start -2.286 0.508)
					(end 2.286 1.016)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy -1.778 2.286) (xy -0.762 2.286)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy -1.27 2.794) (xy -1.27 1.778)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(rectangle
					(start 2.286 -0.508)
					(end -2.286 -1.016)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type outline)
					)
				)
			)
			(symbol "C_Polarized_1_1"
				(pin passive line
					(at 0 3.81 270)
					(length 2.794)
					(name ""
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 0 -3.81 90)
					(length 2.794)
					(name ""
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Device:D_Schottky"
			(pin_numbers
				(hide yes)
			)
			(pin_names
				(offset 1.016)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "D"
				(at 0 2.54 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "D_Schottky"
				(at 0 -2.54 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Schottky diode"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "diode Schottky"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "TO-???* *_Diode_* *SingleDiode* D_*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "D_Schottky_0_1"
				(polyline
					(pts
						(xy -1.905 0.635) (xy -1.905 1.27) (xy -1.27 1.27) (xy -1.27 -1.27) (xy -0.635 -1.27) (xy -0.635 -0.635)
					)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 1.27 1.27) (xy 1.27 -1.27) (xy -1.27 0) (xy 1.27 1.27)
					)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 1.27 0) (xy -1.27 0)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "D_Schottky_1_1"
				(pin passive line
					(at -3.81 0 0)
					(length 2.54)
					(name "K"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 3.81 0 180)
					(length 2.54)
					(name "A"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Device:L"
			(pin_numbers
				(hide yes)
			)
			(pin_names
				(offset 1.016)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "L"
				(at -1.27 0 90)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "L"
				(at 1.905 0 90)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Inductor"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "inductor choke coil reactor magnetic"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "Choke_* *Coil* Inductor_* L_*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "L_0_1"
				(arc
					(start 0 2.54)
					(mid 0.6323 1.905)
					(end 0 1.27)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(arc
					(start 0 1.27)
					(mid 0.6323 0.635)
					(end 0 0)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(arc
					(start 0 0)
					(mid 0.6323 -0.635)
					(end 0 -1.27)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(arc
					(start 0 -1.27)
					(mid 0.6323 -1.905)
					(end 0 -2.54)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "L_1_1"
				(pin passive line
					(at 0 3.81 270)
					(length 1.27)
					(name "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 0 -3.81 90)
					(length 1.27)
					(name "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Device:R"
			(pin_numbers
				(hide yes)
			)
			(pin_names
				(offset 0)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "R"
				(at 2.032 0 90)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "R"
				(at 0 0 90)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at -1.778 0 90)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Resistor"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "R res resistor"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "R_*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "R_0_1"
				(rectangle
					(start -1.016 -2.54)
					(end 1.016 2.54)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "R_1_1"
				(pin passive line
					(at 0 3.81 270)
					(length 1.27)
					(name ""
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 0 -3.81 90)
					(length 1.27)
					(name ""
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "RF_Module:ESP32-WROOM-32E"
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "U"
				(at -12.7 34.29 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Value" "ESP32-WROOM-32E"
				(at 1.27 34.29 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Footprint" "RF_Module:ESP32-WROOM-32E"
				(at 16.51 -34.29 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "RF Module, ESP32-D0WD-V3 SoC, without PSRAM, Wi-Fi 802.11b/g/n, Bluetooth, BLE, 32-bit, 2.7-3.6V, onboard antenna, SMD"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "RF Radio BT ESP ESP32 Espressif onboard PCB antenna"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "ESP32?WROOM?32E*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "ESP32-WROOM-32E_0_1"
				(rectangle
					(start -12.7 33.02)
					(end 12.7 -33.02)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
			)
			(symbol "ESP32-WROOM-32E_1_1"
				(pin power_in line
					(at 0 35.56 270)
					(length 2.54)
					(name "VDD"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -15.24 30.48 0)
					(length 2.54)
					(name "EN"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -15.24 25.4 0)
					(length 2.54)
					(name "SENSOR_VP"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -15.24 22.86 0)
					(length 2.54)
					(name "SENSOR_VN"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at 15.24 -25.4 180)
					(length 2.54)
					(name "IO34"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "6"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at 15.24 -27.94 180)
					(length 2.54)
					(name "IO35"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "7"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 -20.32 180)
					(length 2.54)
					(name "IO32"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "8"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 -22.86 180)
					(length 2.54)
					(name "IO33"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "9"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 -12.7 180)
					(length 2.54)
					(name "IO25"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "10"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 -15.24 180)
					(length 2.54)
					(name "IO26"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "11"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 -17.78 180)
					(length 2.54)
					(name "IO27"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "12"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 10.16 180)
					(length 2.54)
					(name "IO14"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "13"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 15.24 180)
					(length 2.54)
					(name "IO12"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "14"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 12.7 180)
					(length 2.54)
					(name "IO13"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "16"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin no_connect line
					(at -12.7 -5.08 0)
					(length 2.54)
					(hide yes)
					(name "NC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "17"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin no_connect line
					(at -12.7 -7.62 0)
					(length 2.54)
					(hide yes)
					(name "NC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "18"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin no_connect line
					(at -12.7 -12.7 0)
					(length 2.54)
					(hide yes)
					(name "NC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "19"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin no_connect line
					(at -12.7 -10.16 0)
					(length 2.54)
					(hide yes)
					(name "NC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "20"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin no_connect line
					(at -12.7 0 0)
					(length 2.54)
					(hide yes)
					(name "NC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "21"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin no_connect line
					(at -12.7 -2.54 0)
					(length 2.54)
					(hide yes)
					(name "NC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "22"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 7.62 180)
					(length 2.54)
					(name "IO15"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "23"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 25.4 180)
					(length 2.54)
					(name "IO2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "24"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 30.48 180)
					(length 2.54)
					(name "IO0"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "25"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 20.32 180)
					(length 2.54)
					(name "IO4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "26"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 5.08 180)
					(length 2.54)
					(name "IO16"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "27"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 2.54 180)
					(length 2.54)
					(name "IO17"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "28"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 17.78 180)
					(length 2.54)
					(name "IO5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "29"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 0 180)
					(length 2.54)
					(name "IO18"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "30"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 -2.54 180)
					(length 2.54)
					(name "IO19"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "31"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin no_connect line
					(at -12.7 -27.94 0)
					(length 2.54)
					(hide yes)
					(name "NC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "32"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 -5.08 180)
					(length 2.54)
					(name "IO21"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "33"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 22.86 180)
					(length 2.54)
					(name "RXD0/IO3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "34"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 27.94 180)
					(length 2.54)
					(name "TXD0/IO1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "35"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 -7.62 180)
					(length 2.54)
					(name "IO22"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "36"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 15.24 -10.16 180)
					(length 2.54)
					(name "IO23"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "37"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at 0 -35.56 90)
					(length 2.54)
					(name "GND"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "[1,15,38,39]"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Regulator_Linear:AP2112K-3.3"
			(pin_names
				(offset 0.254)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "U"
				(at -5.08 5.715 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Value" "AP2112K-3.3"
				(at 0 5.715 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Footprint" "Package_TO_SOT_SMD:SOT-23-5"
				(at 0 8.255 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" "https://www.diodes.com/assets/Datasheets/AP2112.pdf"
				(at 0 2.54 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "600mA low dropout linear regulator, with enable pin, 3.8V-6V input voltage range, 3.3V fixed positive output, SOT-23-5"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "linear regulator ldo fixed positive"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "SOT?23?5*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "AP2112K-3.3_0_1"
				(rectangle
					(start -5.08 4.445)
					(end 5.08 -5.08)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
			)
			(symbol "AP2112K-3.3_1_1"
				(pin power_in line
					(at -7.62 2.54 0)
					(length 2.54)
					(name "VIN"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at 0 -7.62 90)
					(length 2.54)
					(name "GND"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -7.62 0 0)
					(length 2.54)
					(name "EN"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin no_connect line
					(at 5.08 0 180)
					(length 2.54)
					(hide yes)
					(name "NC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_out line
					(at 7.62 2.54 180)
					(length 2.54)
					(name "VOUT"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Regulator_Switching:TPS5430DDA"
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "U"
				(at -10.16 8.89 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Value" "TPS5430DDA"
				(at -1.27 8.89 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Footprint" "Package_SO:TI_SO-PowerPAD-8_ThermalVias"
				(at 1.27 -8.89 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
						(italic yes)
					)
					(justify left)
				)
			)
			(property "Datasheet" "http://www.ti.com/lit/ds/symlink/tps5430.pdf"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "3A, Step Down Swift Converter, Adjustable Output Voltage, 5.5-36V Input Voltage, PowerSO-8"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "Step-Down DC-DC Switching Regulator"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "TI*SO*PowerPAD*ThermalVias*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "TPS5430DDA_0_1"
				(rectangle
					(start -10.16 7.62)
					(end 10.16 -7.62)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
			)
			(symbol "TPS5430DDA_1_1"
				(pin input line
					(at 12.7 5.08 180)
					(length 2.54)
					(name "BOOT"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin no_connect line
					(at -10.16 2.54 0)
					(length 2.54)
					(hide yes)
					(name "NC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin no_connect line
					(at -10.16 -2.54 0)
					(length 2.54)
					(hide yes)
					(name "NC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "3"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at 12.7 -5.08 180)
					(length 2.54)
					(name "VSENSE"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "4"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -12.7 -5.08 0)
					(length 2.54)
					(name "EN"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "5"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at -2.54 -10.16 90)
					(length 2.54)
					(name "GND"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "6"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at -12.7 5.08 0)
					(length 2.54)
					(name "VIN"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "7"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 12.7 0 180)
					(length 2.54)
					(name "PH"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "8"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at 0 -10.16 90)
					(length 2.54)
					(name "GNDPAD"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "9"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Sensor_Motion:MPU-9250"
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "U"
				(at -11.43 19.05 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "MPU-9250"
				(at 7.62 -19.05 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" "Sensor_Motion:InvenSense_QFN-24_3x3mm_P0.4mm"
				(at 0 -25.4 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" "https://invensense.tdk.com/wp-content/uploads/2015/02/PS-MPU-9250A-01-v1.1.pdf"
				(at 0 -3.81 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "InvenSense 9-Axis Motion Sensor, Accelerometer, Gyroscope, Compass, I2C/SPI"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "mems magnetometer"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_fp_filters" "*QFN?24*3x3mm*P0.4mm*"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "MPU-9250_0_1"
				(rectangle
					(start -12.7 17.78)
					(end 12.7 -17.78)
					(stroke
						(width 0.254)
						(type default)
					)
					(fill
						(type background)
					)
				)
			)
			(symbol "MPU-9250_1_1"
				(pin input line
					(at 17.78 -7.62 180)
					(length 5.08)
					(name "RESV_VDDIO"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output clock
					(at 17.78 0 180)
					(length 5.08)
					(name "AUX_CL"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "7"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at -2.54 22.86 270)
					(length 5.08)
					(name "VDDIO"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "8"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at -17.78 5.08 0)
					(length 5.08)
					(name "AD0/MISO"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "9"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 17.78 -5.08 180)
					(length 5.08)
					(name "REGOUT"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "10"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -17.78 -5.08 0)
					(length 5.08)
					(name "FSYNC"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "11"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin output line
					(at 17.78 7.62 180)
					(length 5.08)
					(name "INT"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "12"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at 2.54 22.86 270)
					(length 5.08)
					(name "VDD"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "13"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at 0 -22.86 90)
					(length 5.08)
					(name "GND"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "18"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin power_in line
					(at 17.78 -10.16 180)
					(length 5.08)
					(name "RESV_GND"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "20"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at 17.78 2.54 180)
					(length 5.08)
					(name "AUX_DA"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "21"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -17.78 0 0)
					(length 5.08)
					(name "~{CS}"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "22"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin input line
					(at -17.78 2.54 0)
					(length 5.08)
					(name "SCL/SCLK"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "23"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin bidirectional line
					(at -17.78 7.62 0)
					(length 5.08)
					(name "SDA/MOSI"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "24"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Switch:SW_Push"
			(pin_numbers
				(hide yes)
			)
			(pin_names
				(offset 1.016)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "SW"
				(at 1.27 2.54 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
					(justify left)
				)
			)
			(property "Value" "SW_Push"
				(at 0 -1.524 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 5.08 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 5.08 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Push button switch, generic, two pins"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "switch normally-open pushbutton push-button"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "SW_Push_0_1"
				(circle
					(center -2.032 0)
					(radius 0.508)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 0 1.27) (xy 0 3.048)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(circle
					(center 2.032 0)
					(radius 0.508)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 2.54 1.27) (xy -2.54 1.27)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(pin passive line
					(at -5.08 0 0)
					(length 2.54)
					(name "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 5.08 0 180)
					(length 2.54)
					(name "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "Switch:SW_SPST"
			(pin_names
				(offset 0)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "SW"
				(at 0 3.175 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "SW_SPST"
				(at 0 -2.54 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Single Pole Single Throw (SPST) switch"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "switch lever"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "SW_SPST_0_0"
				(circle
					(center -2.032 0)
					(radius 0.508)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy -1.524 0.254) (xy 1.524 1.778)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(circle
					(center 2.032 0)
					(radius 0.508)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "SW_SPST_1_1"
				(pin passive line
					(at -5.08 0 0)
					(length 2.54)
					(name "A"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
				(pin passive line
					(at 5.08 0 180)
					(length 2.54)
					(name "B"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "2"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "power:+3V3"
			(power global)
			(pin_numbers
				(hide yes)
			)
			(pin_names
				(offset 0)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "#PWR"
				(at 0 -3.81 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "+3V3"
				(at 0 3.556 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Power symbol creates a global label with name \"+3V3\""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "global power"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "+3V3_0_1"
				(polyline
					(pts
						(xy -0.762 1.27) (xy 0 2.54)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 0 2.54) (xy 0.762 1.27)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 0 0) (xy 0 2.54)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "+3V3_1_1"
				(pin power_in line
					(at 0 0 90)
					(length 0)
					(name ""
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "power:+5V"
			(power global)
			(pin_numbers
				(hide yes)
			)
			(pin_names
				(offset 0)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "#PWR"
				(at 0 -3.81 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "+5V"
				(at 0 3.556 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Power symbol creates a global label with name \"+5V\""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "global power"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "+5V_0_1"
				(polyline
					(pts
						(xy -0.762 1.27) (xy 0 2.54)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 0 2.54) (xy 0.762 1.27)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
				(polyline
					(pts
						(xy 0 0) (xy 0 2.54)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "+5V_1_1"
				(pin power_in line
					(at 0 0 90)
					(length 0)
					(name ""
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "power:GND"
			(power global)
			(pin_numbers
				(hide yes)
			)
			(pin_names
				(offset 0)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "#PWR"
				(at 0 -6.35 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "GND"
				(at 0 -3.81 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Power symbol creates a global label with name \"GND\" , ground"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "global power"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "GND_0_1"
				(polyline
					(pts
						(xy 0 0) (xy 0 -1.27) (xy 1.27 -1.27) (xy 0 -2.54) (xy -1.27 -1.27) (xy 0 -1.27)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(symbol "GND_1_1"
				(pin power_in line
					(at 0 0 270)
					(length 0)
					(name ""
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(embedded_fonts no)
		)
		(symbol "power:PWR_FLAG"
			(power global)
			(pin_numbers
				(hide yes)
			)
			(pin_names
				(offset 0)
				(hide yes)
			)
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(in_pos_files yes)
			(duplicate_pin_numbers_are_jumpers no)
			(property "Reference" "#FLG"
				(at 0 1.905 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Value" "PWR_FLAG"
				(at 0 3.81 0)
				(show_name no)
				(do_not_autoplace no)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Footprint" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Datasheet" ""
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "Description" "Special symbol for telling ERC where power comes from"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(property "ki_keywords" "flag power"
				(at 0 0 0)
				(show_name no)
				(do_not_autoplace no)
				(hide yes)
				(effects
					(font
						(size 1.27 1.27)
					)
				)
			)
			(symbol "PWR_FLAG_0_0"
				(pin power_out line
					(at 0 0 90)
					(length 0)
					(name ""
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
					(number "1"
						(effects
							(font
								(size 1.27 1.27)
							)
						)
					)
				)
			)
			(symbol "PWR_FLAG_0_1"
				(polyline
					(pts
						(xy 0 0) (xy 0 1.27) (xy -1.016 1.905) (xy 0 2.54) (xy 1.016 1.905) (xy 0 1.27)
					)
					(stroke
						(width 0)
						(type default)
					)
					(fill
						(type none)
					)
				)
			)
			(embedded_fonts no)
		)
	)
	(text "1. Inspect assembly, then apply raw 3S input from a current-limited bench supply."
		(exclude_from_sim no)
		(at 254 214.63 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "033906ba-4a65-4c50-b60b-5ba69f03b172")
	)
	(text "Architecture: onboard ESP32-WROOM-32E, MPU-9250, 3S-to-5V buck, 3.3V LDO, GPS, iBUS and four PWM ESC outputs."
		(exclude_from_sim no)
		(at 16.51 24.13 0)
		(effects
			(font
				(size 1.524 1.524)
				(thickness 0.3048)
				(bold yes)
			)
			(justify left bottom)
		)
		(uuid "13307160-da81-4ace-9458-ee6cc11760a8")
	)
	(text "Place C10/C11/C12 within a few mm of U2. Keep U2 away from high-current motor and BEC paths."
		(exclude_from_sim no)
		(at 317.5 43.18 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "2f212b7c-ea1d-4864-99f3-6bdd440f02ff")
	)
	(text "ASSEMBLY / BRING-UP NOTES"
		(exclude_from_sim no)
		(at 254 208.28 0)
		(effects
			(font
				(size 2.032 2.032)
				(thickness 0.4064)
				(bold yes)
			)
			(justify left bottom)
		)
		(uuid "38d108d6-781d-468b-8511-f6e664ce9c32")
	)
	(text "BMP280 and optional VL53L1X share this bus."
		(exclude_from_sim no)
		(at 311.15 83.82 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "537cd9cd-3f01-4cae-96f5-489ddaea97cb")
	)
	(text "SW2 is a latching slide switch; OFF pulls TPS5430 EN low through R14."
		(exclude_from_sim no)
		(at 41.91 71.12 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "55e0b817-4670-464e-b096-c28f5768c096")
	)
	(text "FOUR PWM ESC OUTPUTS — SIGNAL AND GROUND ONLY"
		(exclude_from_sim no)
		(at 16.51 107.95 0)
		(effects
			(font
				(size 2.032 2.032)
				(thickness 0.4064)
				(bold yes)
			)
			(justify left bottom)
		)
		(uuid "590f3720-2de6-4870-8881-c1726d2a19ed")
	)
	(text "3. Confirm MPU-9250 WHO_AM_I and BMP280 detection before connecting ESCs."
		(exclude_from_sim no)
		(at 254 224.79 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "59d7c850-6263-4176-bd01-882c128dd97b")
	)
	(text "Firmware source: https://github.com/durveshpathak1989/ESP32-FlightStack"
		(exclude_from_sim no)
		(at 254 241.3 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "74285ec3-6dc9-49bd-9caf-3513051265f4")
	)
	(text "4. First motor tests must be performed without propellers."
		(exclude_from_sim no)
		(at 254 229.87 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "7dc136f4-d04c-44af-9b42-c76f92f6fe42")
	)
	(text "Motor map: FL GPIO25 (CCW), FR GPIO4 (CW), RL GPIO26 (CW), RR GPIO32 (CCW)."
		(exclude_from_sim no)
		(at 16.51 165.1 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "7e00f3cc-b3e3-4831-9e11-13129e719f38")
	)
	(text "2. Confirm +3V3, then verify the battery ADC node with a multimeter."
		(exclude_from_sim no)
		(at 254 219.71 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "9349b4c8-1229-4ccb-b229-f1258c801517")
	)
	(text "ESP32 FLIGHTSTACK - INTEGRATED 4-LAYER FLIGHT CONTROLLER"
		(exclude_from_sim no)
		(at 16.51 17.78 0)
		(effects
			(font
				(size 3.048 3.048)
				(thickness 0.6096)
				(bold yes)
			)
			(justify left bottom)
		)
		(uuid "a6923728-e140-4a4d-a9db-761bd5dadfca")
	)
	(text "POWER SYSTEM"
		(exclude_from_sim no)
		(at 16.51 35.56 0)
		(effects
			(font
				(size 2.032 2.032)
				(thickness 0.4064)
				(bold yes)
			)
			(justify left bottom)
		)
		(uuid "a7af485b-f163-49e4-8119-d2bab2608317")
	)
	(text "SAFETY: Raw 3S LiPo input only. First power-up must use a current-limited bench supply; test motors without propellers."
		(exclude_from_sim no)
		(at 16.51 28.575 0)
		(effects
			(font
				(size 1.524 1.524)
				(thickness 0.3048)
				(bold yes)
			)
			(justify left bottom)
		)
		(uuid "a8784669-d677-42af-ba73-daf13e38794a")
	)
	(text "FSYNC is grounded. AUX_DA/AUX_CL are unused. Pin 1 follows datasheet: reserved, tied to VDDIO."
		(exclude_from_sim no)
		(at 317.5 48.26 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "aafb0f93-4195-41a3-970c-6e62390221ef")
	)
	(text "ESC PORTS: signal + ground only. Do not connect ESC BEC outputs."
		(exclude_from_sim no)
		(at 16.51 175.26 0)
		(effects
			(font
				(size 1.524 1.524)
				(thickness 0.3048)
				(bold yes)
			)
			(justify left bottom)
		)
		(uuid "bd277b0f-e2a0-450d-bc93-3e692fb5684d")
	)
	(text "JTAG uses GPIO12-15 exclusively; flight I/O is remapped."
		(exclude_from_sim no)
		(at 182.88 256.54 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "c2df97cd-7872-4868-aecd-86c3c9c1cff4")
	)
	(text "5. Document the PCB/IMU axis orientation in firmware before flight testing."
		(exclude_from_sim no)
		(at 254 234.95 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "caa71b94-26cb-4b07-ad7f-e0f1a0339ee0")
	)
	(text "Confirm receiver signal is 3.3 V logic before connection."
		(exclude_from_sim no)
		(at 289.56 135.89 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "d003f6f4-14b0-40e4-8b30-2c691b167c0d")
	)
	(text "GPS TXD -> GPS_RX (GPIO23); GPS RXD <- GPS_TX (GPIO17)"
		(exclude_from_sim no)
		(at 289.56 113.03 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "dd610809-79a1-4077-a365-fc2c592a1466")
	)
	(text "INTEGRATED MPU-9250 IMU (SPI)"
		(exclude_from_sim no)
		(at 254 35.56 0)
		(effects
			(font
				(size 2.032 2.032)
				(thickness 0.4064)
				(bold yes)
			)
			(justify left bottom)
		)
		(uuid "e63ed3ab-c243-4bd2-bdde-4926c03b6a54")
	)
	(text "ONBOARD 3S TO 5V BUCK (3A)"
		(exclude_from_sim no)
		(at 57.15 36.83 0)
		(effects
			(font
				(size 1.524 1.524)
				(thickness 0.3048)
				(bold yes)
			)
			(justify left bottom)
		)
		(uuid "f4849610-69ac-449c-b2e4-648025d4deba")
	)
	(junction
		(at 163.83 60.96)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "09b1fd03-edc6-42e1-bb7e-6c8edef5bffb")
	)
	(junction
		(at 297.18 77.47)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "11779d5f-193b-408d-b822-ec5debe139c8")
	)
	(junction
		(at 186.69 60.96)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "1b282e9c-db19-472d-acf5-d55e4d6d042f")
	)
	(junction
		(at 62.23 71.12)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "1f48d67d-fe69-43f1-96b0-f346ead9a595")
	)
	(junction
		(at 190.5 165.1)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "317bcad4-550b-4eb2-947a-87d0af11bf15")
	)
	(junction
		(at 83.82 53.34)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "442a2a0e-1c11-4a4e-8888-1e39e1619c3a")
	)
	(junction
		(at 284.48 36.83)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "4ec3c828-32a1-45ee-a22f-91c6d0deda79")
	)
	(junction
		(at 104.14 48.26)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "599f55db-d747-4ece-9f0e-248ad56f1a5b")
	)
	(junction
		(at 167.64 60.96)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "5d30bef3-41b7-43ad-b265-80941e43180f")
	)
	(junction
		(at 104.14 52.07)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "608a8d21-b1c2-438e-befb-89e2bed1d118")
	)
	(junction
		(at 175.26 74.93)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "78c2f6bd-0dbf-4c42-bf1e-a7c71dae17bb")
	)
	(junction
		(at 279.4 36.83)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "81520028-093c-448d-9439-f6812f74c5c0")
	)
	(junction
		(at 57.15 53.34)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "8ed758e9-8482-401e-bc06-3f0c644e6762")
	)
	(junction
		(at 68.58 60.96)
		(diameter 0)
		(color 0 0 0 0)
		(uuid "ada7f191-84e8-4277-b344-d9c15615b9f4")
	)
	(no_connect
		(at 299.72 58.42)
		(uuid "7336bf79-950a-400d-9600-0aef007e7e4e")
	)
	(no_connect
		(at 213.36 154.94)
		(uuid "819dd1d7-adc3-4445-ae3f-e7d75528caeb")
	)
	(no_connect
		(at 299.72 60.96)
		(uuid "91d7f9cd-82f7-4f79-bc2a-860c1b83ec02")
	)
	(no_connect
		(at 213.36 101.6)
		(uuid "ab59f5c8-c40e-45b1-b3b5-32c5fca6c18a")
	)
	(no_connect
		(at 182.88 101.6)
		(uuid "ac797861-5669-495c-9a10-f41a856fd742")
	)
	(no_connect
		(at 182.88 104.14)
		(uuid "e0e70e87-1640-468d-a054-081f8668a96d")
	)
	(wire
		(pts
			(xy 266.7 93.98) (xy 274.32 93.98)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "014e93ef-fa84-49b9-a272-99655ee87791")
	)
	(wire
		(pts
			(xy 163.83 60.96) (xy 163.83 64.77)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "02ca8004-29db-4905-bebf-8869e2910cc6")
	)
	(wire
		(pts
			(xy 83.82 48.26) (xy 92.71 48.26)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0457a7d5-a921-4456-8472-0eb0ea3711ea")
	)
	(wire
		(pts
			(xy 181.61 173.99) (xy 181.61 175.26)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "04e4bf8c-a070-4bfb-83f9-67f3d950cbec")
	)
	(wire
		(pts
			(xy 81.28 160.02) (xy 81.28 162.56)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "04f6de74-4982-4998-9866-0791f4bbd35f")
	)
	(wire
		(pts
			(xy 311.15 73.66) (xy 311.15 74.93)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0b9eac54-3e47-4640-829f-b69ea85e7176")
	)
	(wire
		(pts
			(xy 83.82 53.34) (xy 81.28 53.34)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0dabd908-38aa-4881-bbb0-ee572860691f")
	)
	(wire
		(pts
			(xy 279.4 114.3) (xy 279.4 113.03)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0dc8fb1f-893b-4f96-9f02-68bf1874d6ee")
	)
	(wire
		(pts
			(xy 297.18 77.47) (xy 303.53 77.47)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "0fa07856-a341-4998-93db-cbea2fc715b8")
	)
	(wire
		(pts
			(xy 31.75 73.66) (xy 35.56 73.66)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "10dffcba-44db-40ba-ae05-67f6fbf4e7e1")
	)
	(wire
		(pts
			(xy 83.82 43.18) (xy 80.01 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "11f49b5d-0608-4d31-9b5a-a51d89e44b49")
	)
	(wire
		(pts
			(xy 35.56 49.53) (xy 35.56 53.34)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "125c2596-697f-405e-855b-98827cb5adbf")
	)
	(wire
		(pts
			(xy 266.7 135.89) (xy 279.4 135.89)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1abb98ef-ef29-406c-9f58-b05cddbfb2fb")
	)
	(wire
		(pts
			(xy 45.72 118.11) (xy 46.99 118.11)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1ad603f7-27cd-4334-b4c7-c4040df845a8")
	)
	(wire
		(pts
			(xy 203.2 194.31) (xy 203.2 198.12)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "1fc1fc44-9ee9-4754-a557-516ac7ef7bfa")
	)
	(wire
		(pts
			(xy 208.28 218.44) (xy 208.28 215.9)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "24d42260-314d-4b7a-8b12-f8d203d22bff")
	)
	(wire
		(pts
			(xy 71.12 60.96) (xy 68.58 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "35f682a7-c5fc-4bb3-af86-7cab00483faf")
	)
	(wire
		(pts
			(xy 76.2 118.11) (xy 81.28 118.11)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "37926099-b551-48ee-8a6f-48ade6b52dc1")
	)
	(wire
		(pts
			(xy 55.88 53.34) (xy 58.42 53.34)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "38bc1378-d247-4bfb-99c1-ca5731cc6218")
	)
	(wire
		(pts
			(xy 266.7 91.44) (xy 279.4 91.44)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "38fc2fa2-9d5c-4500-bd04-5373f97bc64f")
	)
	(wire
		(pts
			(xy 104.14 48.26) (xy 104.14 52.07)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3a53c3bd-2d80-473e-8b33-1991d9cf8fc6")
	)
	(wire
		(pts
			(xy 76.2 146.05) (xy 81.28 146.05)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3ab1b1d1-1e8c-4a89-bfd1-b5caacf38df9")
	)
	(wire
		(pts
			(xy 163.83 60.96) (xy 167.64 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3c51be88-5db4-4b9f-87a2-2e7d5eacb077")
	)
	(wire
		(pts
			(xy 100.33 53.34) (xy 104.14 53.34)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "3d99b7af-9c9a-453a-bef5-99daaf48af47")
	)
	(wire
		(pts
			(xy 289.56 77.47) (xy 297.18 77.47)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4455d771-197a-4fdb-854b-e9381d71991a")
	)
	(wire
		(pts
			(xy 57.15 63.5) (xy 57.15 64.77)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "45636553-8245-424e-83b5-63dc3be7f120")
	)
	(wire
		(pts
			(xy 31.75 49.53) (xy 35.56 49.53)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4795092d-9b7c-40ae-8dc8-d3827df87703")
	)
	(wire
		(pts
			(xy 304.8 64.77) (xy 304.8 68.58)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "48713884-0484-4828-843d-807bd8bcfccc")
	)
	(wire
		(pts
			(xy 186.69 60.96) (xy 186.69 57.15)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "49cb87df-9605-43ad-9436-704488c50463")
	)
	(wire
		(pts
			(xy 299.72 66.04) (xy 311.15 66.04)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4a3b5b3f-51b1-469e-a218-2ed47c7ffba3")
	)
	(wire
		(pts
			(xy 190.5 153.67) (xy 190.5 156.21)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4a781050-4fca-4342-a3dc-c8e932fb6471")
	)
	(wire
		(pts
			(xy 186.69 74.93) (xy 186.69 72.39)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4ae0daf9-1450-4757-98d2-40281de2fa7f")
	)
	(wire
		(pts
			(xy 308.61 71.12) (xy 308.61 74.93)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4ba33420-3743-404a-8602-3957a94ee3fe")
	)
	(wire
		(pts
			(xy 76.2 160.02) (xy 81.28 160.02)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4c83cb7d-1f8a-4caa-bfdd-ef4e70d6b73a")
	)
	(wire
		(pts
			(xy 279.4 135.89) (xy 279.4 134.62)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4e203fe4-280b-4569-9bce-18b1658d006f")
	)
	(wire
		(pts
			(xy 45.72 127) (xy 46.99 127)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "4f38d6e0-cb1a-4494-8f6e-78e8b39871a6")
	)
	(wire
		(pts
			(xy 284.48 34.29) (xy 284.48 38.1)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "51e98444-1a00-4592-b7a3-9154a041399a")
	)
	(wire
		(pts
			(xy 92.71 53.34) (xy 83.82 53.34)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "54b7606e-d256-458d-9f78-9091da155909")
	)
	(wire
		(pts
			(xy 57.15 55.88) (xy 57.15 53.34)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "55beb3d8-9121-40ee-abde-89a351132e24")
	)
	(wire
		(pts
			(xy 293.37 44.45) (xy 293.37 46.99)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "56443c4b-71dc-4355-a106-dfe8395121c1")
	)
	(wire
		(pts
			(xy 81.28 54.61) (xy 81.28 53.34)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5820814f-37f7-414b-a79a-e224d1e6ec1f")
	)
	(wire
		(pts
			(xy 175.26 71.12) (xy 175.26 76.2)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5ab31662-8847-42a6-833f-9bb4dab05c87")
	)
	(wire
		(pts
			(xy 279.4 91.44) (xy 279.4 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5b8ace83-56ea-4698-82c3-bb4a88cbaa21")
	)
	(wire
		(pts
			(xy 45.72 144.78) (xy 46.99 144.78)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5d196437-4e58-480a-8d54-8eea53c9db6a")
	)
	(wire
		(pts
			(xy 78.74 71.12) (xy 80.01 71.12)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "5db99424-f5c3-4433-accc-d1709aff7244")
	)
	(wire
		(pts
			(xy 284.48 36.83) (xy 293.37 36.83)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "60d106c5-fa35-4f8a-93a2-0245ff8e8ae7")
	)
	(wire
		(pts
			(xy 88.9 43.18) (xy 88.9 50.8)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "65d5df7d-8592-4035-b2f3-fbb23d362fa2")
	)
	(wire
		(pts
			(xy 208.28 191.77) (xy 208.28 189.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6674f3c9-996f-42e8-8f63-a7bda4049892")
	)
	(wire
		(pts
			(xy 196.85 191.77) (xy 208.28 191.77)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "680ab1c9-d250-461c-83a5-e0ff4af00388")
	)
	(wire
		(pts
			(xy 55.88 71.12) (xy 62.23 71.12)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "6dccdc92-acd1-4863-b58a-4060cd9a86b6")
	)
	(wire
		(pts
			(xy 44.45 53.34) (xy 44.45 55.88)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "70770c5f-c24a-4e3c-8ff1-54a9d2961943")
	)
	(wire
		(pts
			(xy 196.85 218.44) (xy 208.28 218.44)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "709834ce-2e52-484f-a17e-f0e00e520b71")
	)
	(wire
		(pts
			(xy 88.9 58.42) (xy 88.9 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "71192533-37ed-4ac1-81af-a3a808ea65a0")
	)
	(wire
		(pts
			(xy 205.74 153.67) (xy 205.74 156.21)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7154c8f2-2a1f-4276-bac5-14d8d6da13d1")
	)
	(wire
		(pts
			(xy 266.7 116.84) (xy 274.32 116.84)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "72557c3a-3c24-4be3-a50e-47be0bbc0e5a")
	)
	(wire
		(pts
			(xy 279.4 34.29) (xy 279.4 38.1)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "74b416b0-dba6-4b1b-a789-d8f1c53cb403")
	)
	(wire
		(pts
			(xy 289.56 80.01) (xy 289.56 77.47)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7b2fae0d-4bb7-4aef-82a8-28826abcc0ec")
	)
	(wire
		(pts
			(xy 80.01 83.82) (xy 80.01 87.63)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7ba4e3d0-462d-4d30-b8d8-cad220bb916a")
	)
	(wire
		(pts
			(xy 76.2 132.08) (xy 81.28 132.08)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7d4e4aba-5e57-409f-841c-d4552670159d")
	)
	(wire
		(pts
			(xy 190.5 163.83) (xy 190.5 165.1)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "7d76790e-5dd6-45fa-9454-b189931c2d0b")
	)
	(wire
		(pts
			(xy 45.72 135.89) (xy 46.99 135.89)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8059c657-0672-40b6-ba8e-0df4f8aaaea9")
	)
	(wire
		(pts
			(xy 80.01 71.12) (xy 80.01 76.2)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "82c59fb9-f500-4197-9ba4-7307e5c2f4af")
	)
	(wire
		(pts
			(xy 104.14 52.07) (xy 104.14 53.34)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8438db27-67bc-4dcc-8637-d062f2ce6d69")
	)
	(wire
		(pts
			(xy 190.5 165.1) (xy 190.5 166.37)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "84c1595d-6cd5-4081-b397-281eca5f0385")
	)
	(wire
		(pts
			(xy 266.7 114.3) (xy 279.4 114.3)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "85774fd7-a290-4d62-93e2-a413e5bff1d1")
	)
	(wire
		(pts
			(xy 132.08 52.07) (xy 132.08 55.88)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8a227f92-a9d1-4834-82d3-8b0e502d78c9")
	)
	(wire
		(pts
			(xy 198.12 162.56) (xy 198.12 165.1)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "8ce0d4c4-7048-4ef6-95da-f65a69bbf622")
	)
	(wire
		(pts
			(xy 274.32 93.98) (xy 274.32 99.06)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9228e2dd-70df-4fd5-bb6a-58c0b8ee5d32")
	)
	(wire
		(pts
			(xy 264.16 66.04) (xy 260.35 66.04)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "927f6e34-a02a-4a4a-8040-fa45d9eb1f6d")
	)
	(wire
		(pts
			(xy 36.83 127) (xy 38.1 127)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "94366ae3-56b2-4a07-89ea-6d9edf1a2f35")
	)
	(wire
		(pts
			(xy 269.24 44.45) (xy 269.24 46.99)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "96ab9da8-a58b-4317-a914-948eecac8dec")
	)
	(wire
		(pts
			(xy 81.28 132.08) (xy 81.28 134.62)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "98bde743-acd7-4ce2-be11-b6d99e739bb4")
	)
	(wire
		(pts
			(xy 175.26 74.93) (xy 186.69 74.93)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9d2052ae-679b-476d-8ed7-1c7d5d497d52")
	)
	(wire
		(pts
			(xy 190.5 165.1) (xy 196.85 165.1)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "9eef4307-ec82-4118-9ecd-acfe77027704")
	)
	(wire
		(pts
			(xy 163.83 72.39) (xy 163.83 74.93)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a3e16038-cce0-482d-9f91-5e18016b5586")
	)
	(wire
		(pts
			(xy 71.12 58.42) (xy 71.12 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "a8eef144-1d57-49e5-b1e5-425fe43a8823")
	)
	(wire
		(pts
			(xy 274.32 138.43) (xy 274.32 143.51)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "aab9831d-eaab-46ab-ac02-923b20188e10")
	)
	(wire
		(pts
			(xy 36.83 135.89) (xy 38.1 135.89)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "aeddfb2c-96e5-4bbc-80bc-25419662de59")
	)
	(wire
		(pts
			(xy 196.85 220.98) (xy 203.2 220.98)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b4cb3f0b-09ea-41fa-8e9b-0a13ebbae687")
	)
	(wire
		(pts
			(xy 299.72 71.12) (xy 308.61 71.12)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b5df837f-bd9c-4d95-8e67-38c13390a415")
	)
	(wire
		(pts
			(xy 35.56 73.66) (xy 35.56 77.47)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b78a60d9-3de1-475f-bc9c-f4f3682de2d0")
	)
	(wire
		(pts
			(xy 269.24 36.83) (xy 279.4 36.83)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "b8312e65-1e19-42de-b652-471f74aa68eb")
	)
	(wire
		(pts
			(xy 281.94 83.82) (xy 281.94 87.63)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "bba36558-1f40-4bd3-809e-47f649f8036b")
	)
	(wire
		(pts
			(xy 289.56 87.63) (xy 289.56 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "bbaac017-8ebc-4922-94ec-ea6cea7be3f7")
	)
	(wire
		(pts
			(xy 100.33 48.26) (xy 104.14 48.26)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "bc00976d-68b8-42c3-8ac2-b09000d77a5f")
	)
	(wire
		(pts
			(xy 104.14 59.69) (xy 104.14 62.23)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c1839a70-c8e5-4bc6-b306-e9e4d3ec89f4")
	)
	(wire
		(pts
			(xy 303.53 77.47) (xy 303.53 80.01)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "c4b8ea56-0bf6-4d84-9421-7de03a22b492")
	)
	(wire
		(pts
			(xy 303.53 87.63) (xy 303.53 90.17)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "cb631c98-4864-4992-9f48-50a7a8f4192b")
	)
	(wire
		(pts
			(xy 182.88 60.96) (xy 186.69 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "cd4f24d8-3b95-4f7f-9872-3b67a9a90681")
	)
	(wire
		(pts
			(xy 297.18 74.93) (xy 297.18 77.47)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d09197c7-2267-4a9e-a5d9-4b640e478482")
	)
	(wire
		(pts
			(xy 87.63 43.18) (xy 88.9 43.18)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d469fc49-d54b-4016-869d-a320913528b7")
	)
	(wire
		(pts
			(xy 196.85 194.31) (xy 203.2 194.31)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d6244f2a-ca1e-4361-bf10-51cc12f51ee0")
	)
	(wire
		(pts
			(xy 62.23 71.12) (xy 62.23 76.2)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "d8062f1e-55ce-4608-ae36-71f3a371c5f6")
	)
	(wire
		(pts
			(xy 49.53 55.88) (xy 49.53 59.69)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "db43edb8-2192-408d-ae1f-a1886326421e")
	)
	(wire
		(pts
			(xy 104.14 43.18) (xy 104.14 48.26)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "dd3b4684-f33f-4925-9207-f8a8aa8f4d03")
	)
	(wire
		(pts
			(xy 205.74 163.83) (xy 205.74 166.37)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ddea2fd1-c40c-47de-9b83-4814c0c6ec16")
	)
	(wire
		(pts
			(xy 36.83 144.78) (xy 38.1 144.78)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e0d5a32e-75e6-4ce3-a49e-fef7e83de475")
	)
	(wire
		(pts
			(xy 198.12 87.63) (xy 198.12 91.44)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e2b1d835-b3f0-4e48-97fb-e4df6321c285")
	)
	(wire
		(pts
			(xy 81.28 146.05) (xy 81.28 148.59)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e3323903-9d9f-4a7f-a1c7-44ec9ce46dde")
	)
	(wire
		(pts
			(xy 260.35 66.04) (xy 260.35 68.58)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e395e68a-1879-445a-b76e-a7822d7e86ca")
	)
	(wire
		(pts
			(xy 81.28 62.23) (xy 81.28 64.77)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e6bb29fc-efd9-49b5-a00e-cf2fda66eb0e")
	)
	(wire
		(pts
			(xy 36.83 118.11) (xy 38.1 118.11)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e706dd7f-d53b-4a96-9a5c-cdc21c1f452f")
	)
	(wire
		(pts
			(xy 167.64 60.96) (xy 167.64 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "e72a4df1-5d18-4a0f-b02b-cfe6b92317de")
	)
	(wire
		(pts
			(xy 44.45 44.45) (xy 44.45 45.72)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "eae5065b-e0bf-4ba8-a31f-8e674a19a2d0")
	)
	(wire
		(pts
			(xy 203.2 220.98) (xy 203.2 224.79)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ecd74cac-c4b1-4e3f-a77a-4dbb1801251d")
	)
	(wire
		(pts
			(xy 163.83 57.15) (xy 163.83 60.96)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ecf4d323-4c0f-4f31-948e-fddbf1233ffa")
	)
	(wire
		(pts
			(xy 274.32 116.84) (xy 274.32 121.92)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "ed0a8198-e33b-488f-9b3c-58614e930eb5")
	)
	(wire
		(pts
			(xy 299.72 68.58) (xy 304.8 68.58)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "eda928e6-7c26-43c9-8b57-36d9d6eeec7b")
	)
	(wire
		(pts
			(xy 62.23 83.82) (xy 62.23 87.63)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f1b97e99-4347-41de-84ca-7e6a603f8d25")
	)
	(wire
		(pts
			(xy 81.28 118.11) (xy 81.28 120.65)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f556c3de-13db-4621-83c8-4b0ba503e8ab")
	)
	(wire
		(pts
			(xy 186.69 60.96) (xy 186.69 64.77)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f58fc4a6-f41a-4b5d-8bce-25ce220875b6")
	)
	(wire
		(pts
			(xy 266.7 138.43) (xy 274.32 138.43)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f78391f8-3aa4-48f2-864d-5cb2fe04b0c7")
	)
	(wire
		(pts
			(xy 68.58 58.42) (xy 68.58 63.5)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "f85a534f-8d14-4577-9921-cbad8a33a14f")
	)
	(wire
		(pts
			(xy 46.99 71.12) (xy 48.26 71.12)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "fa2ee41c-783b-4d79-b550-d36103cede8b")
	)
	(wire
		(pts
			(xy 62.23 71.12) (xy 71.12 71.12)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "fa8b65a4-c93c-43ef-9ade-848bccb11207")
	)
	(wire
		(pts
			(xy 163.83 74.93) (xy 175.26 74.93)
		)
		(stroke
			(width 0)
			(type default)
		)
		(uuid "fe14b0e7-1083-46eb-bed8-fcd525768855")
	)
	(label "UART0_TX"
		(at 213.36 99.06 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "005867ee-859d-4004-b525-4f03a1d83ff6")
	)
	(label "GPS_TX"
		(at 266.7 111.76 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "00bfa967-25a1-4960-904c-a0109428cf23")
	)
	(label "GPS_RX"
		(at 213.36 137.16 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "01e76317-32a3-4cbc-bc70-ce25b8199049")
	)
	(label "JTAG_TMS"
		(at 213.36 116.84 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "022a0a03-d30c-4d32-81fe-fbd8ac740870")
	)
	(label "MOTOR_RR_RAW"
		(at 36.83 144.78 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "0374d722-1253-4b81-9fa8-b06f5d7adb75")
	)
	(label "MOTOR_FL"
		(at 76.2 115.57 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "091c99b3-cedd-4b65-909a-9f33da68e9e1")
	)
	(label "BAT_ADC"
		(at 80.01 71.12 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "0a6c7f2e-f617-4c76-9749-ba513bca41c2")
	)
	(label "MOTOR_RR"
		(at 76.2 157.48 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "0cc971f0-0f22-4ff9-aa60-9f19e924d4f7")
	)
	(label "VBAT_RAW"
		(at 44.45 44.45 90)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "13b844a8-37ae-494e-b449-a55eba591011")
	)
	(label "GPS_RX"
		(at 266.7 109.22 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "1e9c93fe-e22b-471f-ac29-130cc153d98e")
	)
	(label "IBUS_RX"
		(at 213.36 121.92 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "2242e5e7-adfb-420d-ab14-500cc1fb1aad")
	)
	(label "VBAT_RAW"
		(at 46.99 71.12 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "2780b4d4-c856-481a-b5a6-3e62ca9d5166")
	)
	(label "JTAG_TMS"
		(at 196.85 213.36 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "29471ffd-8f76-4675-90d3-e72b4ff660f0")
	)
	(label "VBAT_RAW"
		(at 31.75 71.12 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "2bdb166b-2500-4a51-9327-8aa036a53026")
	)
	(label "I2C_SDA"
		(at 213.36 132.08 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "2d717aa3-e563-4159-8d24-549f9910e4c6")
	)
	(label "I2C_SDA"
		(at 289.56 90.17 270)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "2db3b217-b611-4ee2-accd-310b5fe144f2")
	)
	(label "IMU_CS"
		(at 213.36 149.86 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "3358b01c-d172-4a03-a20b-ad8a43512c1c")
	)
	(label "VBAT_RAW"
		(at 49.53 43.18 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "35d650f6-5643-4757-a1c1-78c850b12538")
	)
	(label "BOOT_IO0"
		(at 205.74 166.37 270)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "390735f2-933d-49b6-836b-8e3b6fee1872")
	)
	(label "BAT_ADC"
		(at 213.36 152.4 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "3edb771c-c720-4b5a-9478-8fe5eb83db61")
	)
	(label "IBUS_RX"
		(at 266.7 133.35 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "3fd3717b-a2a3-4fde-b5c8-c172a3c6a896")
	)
	(label "IMU_INT"
		(at 299.72 53.34 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "401926bc-dee0-4bb9-af53-7f96bc35c95d")
	)
	(label "MOTOR_FR_RAW"
		(at 213.36 106.68 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "43f8c7b1-50f1-4222-953f-5866e8049d0c")
	)
	(label "JTAG_TDI"
		(at 196.85 208.28 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "48a919bf-d92e-445e-931f-e16fb5a41f24")
	)
	(label "JTAG_TDO"
		(at 213.36 119.38 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "48cbf495-346a-4267-83a7-82f467f02c8b")
	)
	(label "ESP_EN"
		(at 196.85 186.69 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "4d444529-3e71-41e9-bcf6-ab8e2eea3432")
	)
	(label "VBAT_RAW"
		(at 31.75 46.99 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "5101005e-919b-4cb1-ad37-fd8880dbaee4")
	)
	(label "GPS_TX"
		(at 213.36 124.46 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "550a1a2b-f828-4430-aa15-f54b08ddb5a4")
	)
	(label "SPI_MISO"
		(at 213.36 129.54 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "5623a12d-e71b-4585-b674-6a2ea81f603f")
	)
	(label "SPI_MISO"
		(at 264.16 55.88 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "694ed4ea-01f7-4c77-929c-bb2524e5a41d")
	)
	(label "SW_NODE"
		(at 83.82 48.26 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "6cee06c9-1282-460d-aeb4-612bbaa11428")
	)
	(label "MOTOR_RL_RAW"
		(at 213.36 142.24 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "6d370bdd-2fdf-4782-a196-62aec54e6c87")
	)
	(label "MOTOR_FR_RAW"
		(at 36.83 127 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "6f764994-102b-4b3d-899b-5b524296ed34")
	)
	(label "MOTOR_RR"
		(at 46.99 144.78 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "70912341-4eb1-48d7-84b0-26d342bbf67b")
	)
	(label "SPI_MOSI"
		(at 264.16 53.34 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "8693eed9-6db0-4aa0-8068-042c8071925a")
	)
	(label "I2C_SCL"
		(at 303.53 90.17 270)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "88f4bed3-3b24-4a7b-bcfb-51c7708b989d")
	)
	(label "UART0_TX"
		(at 196.85 181.61 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "8d17e98c-83c9-4224-bbbc-39f31b6c1393")
	)
	(label "MOTOR_RL"
		(at 46.99 135.89 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "8eaa7226-493a-4f55-8554-ee62815d63b9")
	)
	(label "MOTOR_FL_RAW"
		(at 36.83 118.11 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "8f3ef29b-5df0-4d11-b686-7afd5f92f750")
	)
	(label "I2C_SCL"
		(at 266.7 88.9 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "9d42235c-fc63-4cae-a506-1a8d30562245")
	)
	(label "MOTOR_RR_RAW"
		(at 213.36 147.32 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "9e06e2e1-c75b-4ed5-aff6-8db011c1363f")
	)
	(label "UART0_RX"
		(at 196.85 184.15 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "a2f43f7c-7ba1-4a7d-a157-31e9523cd3af")
	)
	(label "JTAG_TCK"
		(at 196.85 210.82 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "a418f41a-738f-4593-bdbc-5ff6bc5aba24")
	)
	(label "SPI_SCK"
		(at 264.16 58.42 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "a4c0ac32-cfd2-4f27-a86b-00f0ce92d924")
	)
	(label "I2C_SCL"
		(at 213.36 134.62 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "a71295cc-ea09-4f71-906a-614ff49384b5")
	)
	(label "SPI_MOSI"
		(at 213.36 127 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "aa555a7a-c3e8-46e6-819f-0b4ae8ed9ff8")
	)
	(label "I2C_SDA"
		(at 266.7 86.36 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "b2ebc960-b21a-4473-ada7-60573bc28c23")
	)
	(label "JTAG_TDI"
		(at 213.36 111.76 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "b86de6a2-2ef6-4f77-bc14-675fee730bee")
	)
	(label "MOTOR_RL_RAW"
		(at 36.83 135.89 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "bb318942-b51c-4d0d-a952-457991538bbc")
	)
	(label "ESP_EN"
		(at 182.88 96.52 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "c14d3b5d-ca44-451c-a642-8662181bd6b1")
	)
	(label "UART0_RX"
		(at 213.36 104.14 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "c3b69a10-45a7-4dcd-85f7-455908b77a78")
	)
	(label "SPI_SCK"
		(at 213.36 109.22 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "c42478f4-3879-42cf-b417-91393d4282dd")
	)
	(label "BOOT_IO0"
		(at 196.85 189.23 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "c647e487-bdb3-48dd-beec-09c23afdeaeb")
	)
	(label "MOTOR_RL"
		(at 76.2 143.51 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "c8d634e7-dcba-48bc-b545-36b3b9afd405")
	)
	(label "IMU_INT"
		(at 213.36 144.78 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "ce1d84ef-cb1e-40d5-b637-c29f6f0008dd")
	)
	(label "VBAT_RAW"
		(at 58.42 43.18 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "d482723b-d73d-4ecb-9a64-0dc79f9d8765")
	)
	(label "ESP_EN"
		(at 190.5 166.37 270)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "d989b34f-0c2d-4ade-bdc7-d2db9da4a595")
	)
	(label "JTAG_TCK"
		(at 213.36 114.3 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "daea129c-c9d0-4f57-883f-2dc8ae3eccb2")
	)
	(label "JTAG_TDO"
		(at 196.85 215.9 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "e393164d-a750-4c60-978c-ef868f9c027a")
	)
	(label "BOOT_IO0"
		(at 213.36 96.52 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "eda67804-d063-4fab-9cbc-36da5f78be33")
	)
	(label "MOTOR_FL"
		(at 46.99 118.11 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "f553d1f2-a90c-4593-821d-549a4e8128c8")
	)
	(label "IMU_CS"
		(at 264.16 60.96 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "f55da089-bf32-470f-8dba-e4ea28f4e2ce")
	)
	(label "VBAT_RAW"
		(at 45.72 53.34 180)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify right bottom)
		)
		(uuid "f56fd84c-1e7c-414e-93f9-9bf37ccfc640")
	)
	(label "MOTOR_FR"
		(at 76.2 129.54 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "fbb63c49-f4a5-42e6-8838-0f3895b31023")
	)
	(label "MOTOR_FL_RAW"
		(at 213.36 139.7 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "fc3be7c2-96bd-40fb-bb1c-601fe3ab647b")
	)
	(label "MOTOR_FR"
		(at 46.99 127 0)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "fd16757b-277f-4516-b957-535071d05c89")
	)
	(label "ESP_EN"
		(at 181.61 163.83 90)
		(effects
			(font
				(size 1.27 1.27)
			)
			(justify left bottom)
		)
		(uuid "fe377f97-feed-49d6-8224-ae8cdf1ea388")
	)
	(symbol
		(lib_id "RF_Module:ESP32-WROOM-32E")
		(at 198.12 127 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000001")
		(property "Reference" "U1"
			(at 198.12 91.9226 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "ESP32-WROOM-32E"
			(at 198.12 94.234 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "RF_Module:ESP32-WROOM-32"
			(at 198.12 165.1 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32e_esp32-wroom-32ue_datasheet_en.pdf"
			(at 190.5 125.73 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 198.12 127 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "32"
			(uuid "3225d330-6c68-43f3-94b7-2d4468dd7fa6")
		)
		(pin "26"
			(uuid "418968b3-5e9d-456e-b0d1-ae46b681ac15")
		)
		(pin "33"
			(uuid "04b08bdc-4ba4-43f9-a0ab-504cc6893f03")
		)
		(pin "34"
			(uuid "9f80152d-dc4b-464b-9868-c1409eafaf2d")
		)
		(pin "35"
			(uuid "085d5944-62ef-422b-a55f-847bb32e47be")
		)
		(pin "36"
			(uuid "c65242be-56fb-45d1-9c84-700d9aa35ede")
		)
		(pin "37"
			(uuid "3cb29b34-5eaa-439e-8d36-6a75acd37c63")
		)
		(pin "25"
			(uuid "12db66c7-86ed-42df-8ac3-e876e2b1c810")
		)
		(pin "[1,15,38,39]"
			(uuid "ac7bbb4c-b1c7-4b61-a629-2f22be44d23b")
		)
		(pin "18"
			(uuid "c2f93ad2-86cb-47eb-8b79-e539340dd0c4")
		)
		(pin "17"
			(uuid "971a65fd-b8f0-4923-a60b-6d0b33a74c42")
		)
		(pin "16"
			(uuid "b3f3922a-62dc-43ec-8382-99a0e526f7f7")
		)
		(pin "14"
			(uuid "878dc143-72c2-40b8-b66b-80fb4ce348a8")
		)
		(pin "13"
			(uuid "a2e57a73-5f28-4e4e-9c40-a34f295372fc")
		)
		(pin "12"
			(uuid "ddbbdb3f-8ed5-41b7-a0fc-a52f04abda4d")
		)
		(pin "11"
			(uuid "e1e0ce10-2b34-484c-88d0-ea361f834abc")
		)
		(pin "10"
			(uuid "db71edc5-6717-406e-8f50-c1990b4f449d")
		)
		(pin "9"
			(uuid "467be590-ff27-44af-bb76-2d301cf7130c")
		)
		(pin "8"
			(uuid "51eebf4b-201b-484b-b096-fab65ae8ce57")
		)
		(pin "7"
			(uuid "e72fc009-5fb3-4828-a801-b22031b3c381")
		)
		(pin "6"
			(uuid "901e9b8f-4d8f-496c-a4db-00ffacd236a8")
		)
		(pin "5"
			(uuid "cde9d655-77dd-409b-920d-a60e92f669e4")
		)
		(pin "4"
			(uuid "2ddff73d-4570-4b9a-84df-9c14b3059f59")
		)
		(pin "3"
			(uuid "49866d6c-a340-438f-aa10-acdef9922aba")
		)
		(pin "2"
			(uuid "ba359536-456f-4292-9064-86841f986250")
		)
		(pin "19"
			(uuid "9865d434-513d-4488-93db-51f4d2ce4780")
		)
		(pin "22"
			(uuid "c14906e9-1f67-4e6a-aea1-e7af8db32ee5")
		)
		(pin "20"
			(uuid "9c8e32b2-2a5f-4e79-ae7f-1c6d360cd43f")
		)
		(pin "21"
			(uuid "39a8a8c4-0517-44a5-95c2-a7e19f19c9f4")
		)
		(pin "23"
			(uuid "17c12c42-7ebf-454a-8eaf-5eb1248bb795")
		)
		(pin "24"
			(uuid "47a4f3dc-ffa0-4abe-b06a-c59fdc9b67ff")
		)
		(pin "27"
			(uuid "df36a874-4bcf-488b-b937-90ab780db2f6")
		)
		(pin "29"
			(uuid "3cbaa0aa-8a91-41e1-8e63-c4753f241c4c")
		)
		(pin "30"
			(uuid "78bd4861-6b7f-497f-bd68-eb9dccf9f4a2")
		)
		(pin "28"
			(uuid "9a459733-e2b5-4a02-8bff-afa67adb2a50")
		)
		(pin "31"
			(uuid "e7f754ce-c746-4dba-b970-80286e7ac408")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "U1")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "U1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 198.12 87.63 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000002")
		(property "Reference" "#PWR0101"
			(at 198.12 91.44 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 198.501 83.2358 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 198.12 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 198.12 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 198.12 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "5de401cb-2908-486e-baef-4dac25e508c1")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0101")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0101")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 198.12 165.1 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000004")
		(property "Reference" "#PWR0103"
			(at 198.12 171.45 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 198.247 169.4942 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 198.12 165.1 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 198.12 165.1 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 198.12 165.1 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "9e6af8aa-8b2a-439e-b18b-e37539a24b62")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0103")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0103")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x02")
		(at 26.67 46.99 0)
		(mirror y)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000010")
		(property "Reference" "J1"
			(at 28.7528 41.4782 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "3S_LIPO_IN"
			(at 28.7528 43.7896 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Connector_JST:JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical"
			(at 26.67 46.99 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 26.67 46.99 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 26.67 46.99 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "afca275a-ff87-4dec-b02f-eab4e515cd1e")
		)
		(pin "2"
			(uuid "b474d348-e50c-41f6-95ab-5854cac93d81")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J1")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 35.56 53.34 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000012")
		(property "Reference" "#PWR0111"
			(at 35.56 59.69 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 35.687 57.7342 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 35.56 53.34 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 35.56 53.34 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 35.56 53.34 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "0833562a-aede-4afc-b27f-d9d7c1c70237")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0111")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0111")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 44.45 49.53 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000013")
		(property "Reference" "C1"
			(at 47.371 48.3616 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10uF/25V X7R"
			(at 47.371 50.673 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_SMD:C_1210_3225Metric"
			(at 45.4152 53.34 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 44.45 49.53 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 44.45 49.53 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "f9fe60f8-a1f5-4158-a0e4-dfa395185904")
		)
		(pin "2"
			(uuid "ee48dfc7-76c3-44bb-a4df-0edf9b80a063")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "C1")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "C1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 44.45 55.88 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000015")
		(property "Reference" "#PWR0113"
			(at 44.45 62.23 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 44.577 60.2742 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 44.45 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 44.45 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 44.45 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "2c37d836-047d-45ba-9470-6bae65718000")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0113")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0113")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x02")
		(at 26.67 71.12 0)
		(mirror y)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000016")
		(property "Reference" "J2"
			(at 28.7528 65.6082 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "3S_BAT_SENSE"
			(at 28.7528 67.9196 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Connector_JST:JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical"
			(at 26.67 71.12 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 26.67 71.12 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 26.67 71.12 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "8004aef3-f6f1-4985-ac43-1c4d55421030")
		)
		(pin "2"
			(uuid "19ed76b5-4b7a-4880-8db1-4b5562669644")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J2")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 35.56 77.47 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000017")
		(property "Reference" "#PWR0114"
			(at 35.56 83.82 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 35.687 81.8642 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 35.56 77.47 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 35.56 77.47 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 35.56 77.47 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "5e3f27d7-18f2-44b3-87de-e34bd6c06075")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0114")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0114")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 52.07 71.12 270)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000018")
		(property "Reference" "R1"
			(at 52.07 65.8622 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "235k 1%"
			(at 52.07 68.1736 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0805_2012Metric"
			(at 52.07 69.342 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 52.07 71.12 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 52.07 71.12 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "8b7550d5-f39a-4a3a-8bf0-28d01ab852dd")
		)
		(pin "2"
			(uuid "93723d91-0795-4086-a35d-38f078c391e7")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R1")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 62.23 80.01 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000019")
		(property "Reference" "R2"
			(at 64.008 78.8416 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "35k 1%"
			(at 64.008 81.153 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0805_2012Metric"
			(at 60.452 80.01 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 62.23 80.01 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 62.23 80.01 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "3733b284-f930-428a-884a-e425524935bb")
		)
		(pin "2"
			(uuid "6da797c2-c0d4-4a2d-8df8-e98dc4d03149")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R2")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 62.23 87.63 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000001a")
		(property "Reference" "#PWR0115"
			(at 62.23 93.98 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 62.357 92.0242 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 62.23 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 62.23 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 62.23 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "7f0572d3-d1f0-4d1d-8cea-e12a18f3d614")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0115")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0115")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 74.93 71.12 270)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000001b")
		(property "Reference" "R3"
			(at 74.93 65.8622 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "1k"
			(at 74.93 68.1736 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0805_2012Metric"
			(at 74.93 69.342 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 74.93 71.12 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 74.93 71.12 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "fab395d3-a46d-4997-9b88-0b5a6e569555")
		)
		(pin "2"
			(uuid "8cd47d7a-f0b1-44b5-9a89-342597ef97ec")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R3")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R3")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 80.01 80.01 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000001c")
		(property "Reference" "C2"
			(at 82.931 78.8416 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "100n"
			(at 82.931 81.153 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_SMD:C_0805_2012Metric"
			(at 80.9752 83.82 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 80.01 80.01 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 80.01 80.01 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "a5ecf429-d9eb-4a81-9559-9e5b3a36f57f")
		)
		(pin "2"
			(uuid "ac2761f7-c76f-42f6-b045-aba35d99d5b2")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "C2")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "C2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 80.01 87.63 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000001d")
		(property "Reference" "#PWR0116"
			(at 80.01 93.98 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 80.137 92.0242 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 80.01 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 80.01 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 80.01 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "3a993d56-a552-436c-ad66-202d3cd7be23")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0116")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0116")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Sensor_Motion:MPU-9250")
		(at 281.94 60.96 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000020")
		(property "Reference" "U2"
			(at 270.51 36.83 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "MPU-9250"
			(at 289.56 36.83 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Sensor_Motion:InvenSense_QFN-24_3x3mm_P0.4mm"
			(at 281.94 86.36 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" "https://product.tdk.com/system/files/dam/doc/product/sensor/mortion-inertial/imu/data_sheet/ps-mpu-9250a-01-v1.1.pdf"
			(at 281.94 70.612 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 281.94 60.96 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "a6868b92-c6db-474c-93fa-f4588aa55057")
		)
		(pin "7"
			(uuid "e66a7ec6-eff5-4b3d-a0ea-30b7b5b77f5d")
		)
		(pin "8"
			(uuid "1138a1cc-7e63-4935-9a05-65161c5cfcc8")
		)
		(pin "9"
			(uuid "80620af9-4850-4257-89c9-63cb05c1295d")
		)
		(pin "10"
			(uuid "6ebdaf5d-2539-4086-ab7b-7fdf03653741")
		)
		(pin "11"
			(uuid "6fe5ddb5-5cf4-4ef2-aab7-98f7827756d3")
		)
		(pin "12"
			(uuid "df78cbc5-9e97-46d2-8e47-7fcee8cb9901")
		)
		(pin "13"
			(uuid "1be911df-f956-43cc-9c31-e26eb10e6895")
		)
		(pin "18"
			(uuid "773c9ae9-a112-4607-9311-bc86c94f1a1c")
		)
		(pin "20"
			(uuid "a63a53fe-f2ed-42de-ab7e-bdc11b93f8a5")
		)
		(pin "21"
			(uuid "cabef165-14e4-468f-b8d8-0385274446c9")
		)
		(pin "22"
			(uuid "32ce2ef9-a1dc-48f7-a5b4-2c30ff8a1604")
		)
		(pin "23"
			(uuid "ee76627f-f256-4bcc-b2f3-a66b4d4c637e")
		)
		(pin "24"
			(uuid "e6a799ca-d401-47a9-a06d-f3ecfab673a2")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "U2")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "U2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 260.35 68.58 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000021")
		(property "Reference" "#PWR0120"
			(at 260.35 74.93 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 260.477 72.9742 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 260.35 68.58 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 260.35 68.58 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 260.35 68.58 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "5db03a46-b1f3-4c5e-8cad-d1857b65b7ea")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0120")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0120")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 304.8 64.77 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000022")
		(property "Reference" "#PWR0121"
			(at 304.8 68.58 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 305.181 60.3758 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 304.8 64.77 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 304.8 64.77 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 304.8 64.77 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "d76115dc-56b3-457a-9017-d4646274809a")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0121")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0121")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x04")
		(at 261.62 91.44 180)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000023")
		(property "Reference" "J4"
			(at 259.588 91.2368 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "I2C_SENSOR_BUS"
			(at 259.588 88.9254 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Connector_JST:JST_XH_B4B-XH-A_1x04_P2.50mm_Vertical"
			(at 261.62 91.44 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 261.62 91.44 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 261.62 91.44 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "eed4b736-fb3f-4b8a-b258-b8d1e6dc7c52")
		)
		(pin "2"
			(uuid "2d0aa261-97c5-4faa-bf92-beb95ed4c3b4")
		)
		(pin "3"
			(uuid "80a4c4d9-0c08-48c6-b2f6-ee68e78170e9")
		)
		(pin "4"
			(uuid "3cb871f2-7cac-40aa-aae2-98d5038435e2")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J4")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J4")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 279.4 90.17 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000024")
		(property "Reference" "#PWR0122"
			(at 279.4 93.98 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 279.781 85.7758 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 279.4 90.17 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 279.4 90.17 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 279.4 90.17 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "3fcefced-61a9-4558-8882-05b4cc4b5540")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0122")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0122")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 274.32 99.06 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000025")
		(property "Reference" "#PWR0123"
			(at 274.32 105.41 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 274.447 103.4542 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 274.32 99.06 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 274.32 99.06 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 274.32 99.06 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "5f03a457-8a54-42fc-b631-f463d416431a")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0123")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0123")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 289.56 83.82 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000026")
		(property "Reference" "R4"
			(at 291.338 82.6516 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "4.7k"
			(at 291.338 84.963 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0805_2012Metric"
			(at 287.782 83.82 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 289.56 83.82 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 289.56 83.82 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "f6e1ba1a-876d-4af7-b7de-a10def616c0b")
		)
		(pin "2"
			(uuid "2bc704a1-df89-479a-b3ca-44f56dbeaf8f")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R4")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R4")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 303.53 83.82 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000027")
		(property "Reference" "R5"
			(at 305.308 82.6516 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "4.7k"
			(at 305.308 84.963 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0805_2012Metric"
			(at 301.752 83.82 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 303.53 83.82 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 303.53 83.82 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "d481715d-46d1-444b-bd23-15fd6df617b6")
		)
		(pin "2"
			(uuid "be9fccca-4e11-4256-a9e0-ad7c02500e57")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R5")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R5")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 297.18 74.93 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000028")
		(property "Reference" "#PWR0124"
			(at 297.18 78.74 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 297.561 70.5358 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 297.18 74.93 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 297.18 74.93 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 297.18 74.93 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "458052eb-166c-486e-b9ed-f6dd34d96380")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0124")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0124")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 308.61 74.93 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000029")
		(property "Reference" "#PWR0125"
			(at 308.61 81.28 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 308.737 79.3242 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 308.61 74.93 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 308.61 74.93 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 308.61 74.93 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "45658a9d-b49d-44be-9296-f6112a7c8791")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0125")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0125")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 279.4 34.29 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000002a")
		(property "Reference" "#PWR0126"
			(at 279.4 38.1 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 279.781 29.8958 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 279.4 34.29 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 279.4 34.29 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 279.4 34.29 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "60e983e4-482a-4692-9669-4b0dd53e2edf")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0126")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0126")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 284.48 34.29 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000002b")
		(property "Reference" "#PWR0127"
			(at 284.48 38.1 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 284.861 29.8958 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 284.48 34.29 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 284.48 34.29 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 284.48 34.29 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "9408a9a5-4d7c-46db-8a8b-7ab82f64034e")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0127")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0127")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 269.24 40.64 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000002c")
		(property "Reference" "C10"
			(at 272.161 39.4716 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10n X7R"
			(at 272.161 41.783 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_SMD:C_0402_1005Metric"
			(at 270.2052 44.45 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 269.24 40.64 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 269.24 40.64 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "998949b5-bd7a-48f4-b891-757380d7c2d9")
		)
		(pin "2"
			(uuid "7b212523-2416-4eaf-b103-65f4aad10a57")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "C10")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "C10")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 293.37 40.64 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000002d")
		(property "Reference" "C11"
			(at 296.291 39.4716 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "100n X7R"
			(at 296.291 41.783 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_SMD:C_0402_1005Metric"
			(at 294.3352 44.45 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 293.37 40.64 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 293.37 40.64 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "0e4a9ba6-cb15-4c40-ba60-a0344e80956c")
		)
		(pin "2"
			(uuid "1591b247-ef61-4ee9-aed2-2aa55dea4da4")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "C11")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "C11")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 269.24 46.99 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000002e")
		(property "Reference" "#PWR0128"
			(at 269.24 53.34 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 269.367 51.3842 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 269.24 46.99 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 269.24 46.99 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 269.24 46.99 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "7e75f271-44c3-42f8-aeb7-167b6f342636")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0128")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0128")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 293.37 46.99 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000002f")
		(property "Reference" "#PWR0129"
			(at 293.37 53.34 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 293.497 51.3842 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 293.37 46.99 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 293.37 46.99 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 293.37 46.99 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "69254b93-b8b5-4351-ab4f-b5781a632191")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0129")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0129")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x04")
		(at 261.62 114.3 180)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000030")
		(property "Reference" "J5"
			(at 259.588 114.0968 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "GPS_NEO6M"
			(at 259.588 111.7854 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Connector_JST:JST_XH_B4B-XH-A_1x04_P2.50mm_Vertical"
			(at 261.62 114.3 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 261.62 114.3 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 261.62 114.3 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "820b0de9-487b-4754-b126-416ffc2c40b2")
		)
		(pin "2"
			(uuid "7baaf22d-f994-4c45-ac05-6f8c14ac0e15")
		)
		(pin "3"
			(uuid "b485cd4f-2a5a-456e-a905-c0a074e2b0f0")
		)
		(pin "4"
			(uuid "96e05f19-5994-4a3f-9470-b6eaede745bf")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J5")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J5")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 279.4 113.03 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000031")
		(property "Reference" "#PWR0130"
			(at 279.4 116.84 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 279.781 108.6358 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 279.4 113.03 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 279.4 113.03 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 279.4 113.03 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "55888baa-df9f-43f9-a77e-922e1793c29a")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0130")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0130")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 274.32 121.92 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000032")
		(property "Reference" "#PWR0131"
			(at 274.32 128.27 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 274.447 126.3142 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 274.32 121.92 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 274.32 121.92 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 274.32 121.92 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "e794aaa5-c5f4-49bb-8be4-972d7d36c192")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0131")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0131")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x03")
		(at 261.62 135.89 180)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000033")
		(property "Reference" "J6"
			(at 259.588 136.9568 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "FLYSKY_IBUS"
			(at 259.588 134.6454 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Connector_JST:JST_XH_B3B-XH-A_1x03_P2.50mm_Vertical"
			(at 261.62 135.89 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 261.62 135.89 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 261.62 135.89 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "d872cf6b-909a-4f68-8632-87a92fec7038")
		)
		(pin "2"
			(uuid "0f9f4508-f042-4378-ba90-4dc4fefb3c09")
		)
		(pin "3"
			(uuid "aa7ec661-0095-431d-9b98-cc1260cad8fe")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J6")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J6")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+5V")
		(at 279.4 134.62 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000034")
		(property "Reference" "#PWR0132"
			(at 279.4 138.43 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+5V"
			(at 279.781 130.2258 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 279.4 134.62 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 279.4 134.62 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 279.4 134.62 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "cad5c904-d51b-492a-8e0e-f455afd37bdd")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0132")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0132")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 274.32 143.51 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000035")
		(property "Reference" "#PWR0133"
			(at 274.32 149.86 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 274.447 147.9042 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 274.32 143.51 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 274.32 143.51 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 274.32 143.51 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "fbec90ae-2c95-4ed8-8fec-ba57bf6d3bf2")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0133")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0133")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 311.15 69.85 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000036")
		(property "Reference" "C12"
			(at 314.071 68.6816 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "100n X7R"
			(at 314.071 70.993 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_SMD:C_0402_1005Metric"
			(at 312.1152 73.66 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 311.15 69.85 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 311.15 69.85 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "87b254c6-6b0d-4adc-8ee9-3b9243035b84")
		)
		(pin "2"
			(uuid "5a077906-f77a-4470-9971-5754ba7705ff")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "C12")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "C12")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 311.15 74.93 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000037")
		(property "Reference" "#PWR0134"
			(at 311.15 81.28 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 311.277 79.3242 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 311.15 74.93 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 311.15 74.93 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 311.15 74.93 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "4f77c84d-15e7-44f2-8ab1-42810a84ab1c")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0134")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0134")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 281.94 87.63 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000038")
		(property "Reference" "#PWR0135"
			(at 281.94 93.98 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 282.067 92.0242 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 281.94 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 281.94 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 281.94 87.63 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "987ee30e-5a91-456c-a593-5fbcd25059df")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0135")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0135")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 41.91 118.11 270)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000040")
		(property "Reference" "R10"
			(at 41.91 112.8522 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "220"
			(at 41.91 115.1636 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0805_2012Metric"
			(at 41.91 116.332 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 41.91 118.11 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 41.91 118.11 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "7304d1de-7e39-4786-9a37-fe3dc990ffb5")
		)
		(pin "2"
			(uuid "9b8c9d75-52f4-4d4e-8680-9efa07da0dab")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R10")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R10")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 41.91 127 270)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000041")
		(property "Reference" "R11"
			(at 41.91 121.7422 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "220"
			(at 41.91 124.0536 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0805_2012Metric"
			(at 41.91 125.222 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 41.91 127 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 41.91 127 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "0f9fddb6-4402-4647-8204-6758c6d76b4e")
		)
		(pin "2"
			(uuid "91baca65-04af-46e7-bcc6-541f00bf312d")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R11")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R11")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 41.91 135.89 270)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000042")
		(property "Reference" "R12"
			(at 41.91 130.6322 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "220"
			(at 41.91 132.9436 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0805_2012Metric"
			(at 41.91 134.112 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 41.91 135.89 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 41.91 135.89 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "b54d75dc-bf45-4f2f-a1ae-34a396630de6")
		)
		(pin "2"
			(uuid "43c5fc6f-28a6-48a8-b330-0747d5ca9448")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R12")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R12")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 41.91 144.78 270)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000043")
		(property "Reference" "R13"
			(at 41.91 139.5222 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "220"
			(at 41.91 141.8336 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0805_2012Metric"
			(at 41.91 143.002 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 41.91 144.78 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 41.91 144.78 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "db70f624-56e9-49cc-b9bd-d94a3c174b6c")
		)
		(pin "2"
			(uuid "95d77677-747d-42a2-848c-7a0353126321")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R13")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R13")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x02")
		(at 71.12 118.11 180)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000044")
		(property "Reference" "J10"
			(at 69.088 117.9068 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "ESC_FL"
			(at 69.088 115.5954 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Connector_JST:JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical"
			(at 71.12 118.11 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 71.12 118.11 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 71.12 118.11 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "9e0b1fae-228c-443a-a0e4-e892ecfc55b2")
		)
		(pin "2"
			(uuid "e248f3c3-995f-46d5-aa9b-4aa7abd2ddae")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J10")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J10")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 81.28 120.65 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000045")
		(property "Reference" "#PWR0140"
			(at 81.28 127 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 81.407 125.0442 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 81.28 120.65 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 81.28 120.65 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 81.28 120.65 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "e8a39721-8b9f-41ff-9342-bec4b06e4d7e")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0140")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0140")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x02")
		(at 71.12 132.08 180)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000046")
		(property "Reference" "J11"
			(at 69.088 131.8768 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "ESC_FR"
			(at 69.088 129.5654 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Connector_JST:JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical"
			(at 71.12 132.08 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 71.12 132.08 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 71.12 132.08 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "bff3146d-57d8-41fb-a5a4-09b793894776")
		)
		(pin "2"
			(uuid "e31bae96-e2ac-4bab-8fcd-7e2d229c0eed")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J11")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J11")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 81.28 134.62 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000047")
		(property "Reference" "#PWR0141"
			(at 81.28 140.97 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 81.407 139.0142 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 81.28 134.62 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 81.28 134.62 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 81.28 134.62 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "201b6d9f-2f4a-46f8-9b93-e315b30ce8fb")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0141")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0141")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x02")
		(at 71.12 146.05 180)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000048")
		(property "Reference" "J12"
			(at 69.088 145.8468 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "ESC_RL"
			(at 69.088 143.5354 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Connector_JST:JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical"
			(at 71.12 146.05 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 71.12 146.05 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 71.12 146.05 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "b80ef1f8-b927-462b-9355-a948f0a6940f")
		)
		(pin "2"
			(uuid "188c64a3-07d4-4b06-8066-baf6cb844191")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J12")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J12")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 81.28 148.59 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000049")
		(property "Reference" "#PWR0142"
			(at 81.28 154.94 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 81.407 152.9842 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 81.28 148.59 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 81.28 148.59 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 81.28 148.59 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "8dd68c41-06fa-4f77-9919-710a3bd1f284")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0142")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0142")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x02")
		(at 71.12 160.02 180)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000004a")
		(property "Reference" "J13"
			(at 69.088 159.8168 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "ESC_RR"
			(at 69.088 157.5054 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Connector_JST:JST_XH_B2B-XH-A_1x02_P2.50mm_Vertical"
			(at 71.12 160.02 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 71.12 160.02 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 71.12 160.02 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "171a1f1e-1e92-407c-9c56-a839a7223f07")
		)
		(pin "2"
			(uuid "eb50c1ca-285b-4b27-a0ce-6ab5444acc35")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J13")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J13")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 81.28 162.56 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000004b")
		(property "Reference" "#PWR0143"
			(at 81.28 168.91 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 81.407 166.9542 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 81.28 162.56 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 81.28 162.56 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 81.28 162.56 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "6e820336-769c-4615-8f74-a8245f21423a")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0143")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0143")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Regulator_Switching:TPS5430DDA")
		(at 71.12 48.26 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000060")
		(property "Reference" "U3"
			(at 71.12 36.3982 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "TPS5430DDA"
			(at 71.12 38.7096 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Package_SO:TI_SO-PowerPAD-8_ThermalVias"
			(at 72.39 59.69 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Datasheet" "https://www.ti.com/lit/ds/symlink/tps5430.pdf"
			(at 71.12 48.26 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 71.12 48.26 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "9020dc6d-daee-46f2-9be1-a451a8e456dd")
		)
		(pin "2"
			(uuid "f513b245-0150-4863-8a33-37a702a3b21d")
		)
		(pin "3"
			(uuid "8cea00c6-e9f6-4988-be3a-71220c69ba47")
		)
		(pin "4"
			(uuid "b040931d-78dd-42ce-897d-5bce648b84f4")
		)
		(pin "5"
			(uuid "0d7fa18e-4405-406e-b6ba-f6d9ac415c76")
		)
		(pin "6"
			(uuid "b7776e74-432f-4b3a-85eb-90a0962a07a3")
		)
		(pin "7"
			(uuid "0ee6df2b-8549-4776-9ab3-6bef8539cfd3")
		)
		(pin "8"
			(uuid "1d23c27e-533b-4f24-b8dc-c6bd32eb0ff2")
		)
		(pin "9"
			(uuid "e3dc01c7-2c09-4ae0-b6da-123ae8599ad0")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "U3")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "U3")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 83.82 43.18 270)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000061")
		(property "Reference" "C3"
			(at 83.82 36.7792 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "10n"
			(at 83.82 39.0906 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Capacitor_SMD:C_0603_1608Metric"
			(at 80.01 44.1452 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 83.82 43.18 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 83.82 43.18 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "3db8bac6-179a-4cfb-8c18-693eb8520e4d")
		)
		(pin "2"
			(uuid "e0011afd-5851-41dd-b796-844c40bcfed0")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "C3")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "C3")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:D_Schottky")
		(at 88.9 54.61 270)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000062")
		(property "Reference" "D1"
			(at 90.932 53.4416 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "B340A"
			(at 90.932 55.753 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Diode_SMD:D_SMA"
			(at 88.9 54.61 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 88.9 54.61 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 88.9 54.61 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "a8781157-bc26-4b55-922d-8f1e96107621")
		)
		(pin "2"
			(uuid "d83d2996-faee-478c-a810-7f5382528d51")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "D1")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "D1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 88.9 60.96 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000063")
		(property "Reference" "#PWR0160"
			(at 88.9 67.31 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 89.027 65.3542 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 88.9 60.96 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 88.9 60.96 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 88.9 60.96 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "4835e774-a8b3-4e62-8bf2-1d36395c88be")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0160")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0160")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:L")
		(at 96.52 48.26 90)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000064")
		(property "Reference" "L1"
			(at 96.52 43.434 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "15uH >=4A"
			(at 96.52 45.7454 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Inductor_SMD:L_Bourns-SRU1028_10.0x10.0mm"
			(at 96.52 48.26 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 96.52 48.26 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 96.52 48.26 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "cb68dadd-3462-435a-aa56-e4d703888189")
		)
		(pin "2"
			(uuid "fce37de3-44f7-49f4-a0f1-394293698a2e")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "L1")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "L1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+5V")
		(at 104.14 43.18 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000065")
		(property "Reference" "#PWR0161"
			(at 104.14 46.99 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+5V"
			(at 104.521 38.7858 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 104.14 43.18 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 104.14 43.18 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 104.14 43.18 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "81d424af-eefb-4ea0-8752-7214a8b49754")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0161")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0161")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C_Polarized")
		(at 104.14 55.88 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000066")
		(property "Reference" "C4"
			(at 107.1372 54.7116 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "220uF/10V LOW-ESR"
			(at 107.1372 57.023 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_SMD:CP_Elec_8x10.5"
			(at 105.1052 59.69 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 104.14 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 104.14 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "4292d955-7368-4836-8b3f-c25bf6910fae")
		)
		(pin "2"
			(uuid "5b1fe395-bb7d-4739-8e3e-9d9394bde427")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "C4")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "C4")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 104.14 62.23 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000067")
		(property "Reference" "#PWR0162"
			(at 104.14 68.58 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 104.267 66.6242 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 104.14 62.23 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 104.14 62.23 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 104.14 62.23 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "dde2ba1f-593f-4b4a-97d5-018130390ee9")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0162")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0162")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 96.52 53.34 270)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000068")
		(property "Reference" "R6"
			(at 96.52 48.0822 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "10k 1%"
			(at 96.52 50.3936 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0603_1608Metric"
			(at 96.52 51.562 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 96.52 53.34 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 96.52 53.34 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "803225c3-eadf-40d8-b443-5493d87b9f97")
		)
		(pin "2"
			(uuid "2f23dda0-a066-47c7-a09e-4d267551a70a")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R6")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R6")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 81.28 58.42 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000069")
		(property "Reference" "R7"
			(at 83.058 57.2516 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "3.24k 1%"
			(at 83.058 59.563 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0603_1608Metric"
			(at 79.502 58.42 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 81.28 58.42 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 81.28 58.42 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "41b1fda6-55e4-4398-b2f9-3e0d9e3a44bc")
		)
		(pin "2"
			(uuid "389251ac-984b-4280-9df3-3c0f01b45b61")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R7")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R7")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 81.28 64.77 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000006a")
		(property "Reference" "#PWR0163"
			(at 81.28 71.12 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 81.407 69.1642 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 81.28 64.77 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 81.28 64.77 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 81.28 64.77 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "409bb68c-388e-4611-91c0-393e8a736504")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0163")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0163")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 68.58 63.5 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000006b")
		(property "Reference" "#PWR0164"
			(at 68.58 69.85 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 68.707 67.8942 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 68.58 63.5 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 68.58 63.5 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 68.58 63.5 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "9797a7f0-6702-4c95-9dd2-308403ecfdf4")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0164")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0164")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:PWR_FLAG")
		(at 49.53 43.18 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000006c")
		(property "Reference" "#FLG0160"
			(at 49.53 41.275 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "PWR_FLAG"
			(at 49.53 38.7858 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 49.53 43.18 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 49.53 43.18 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 49.53 43.18 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "d2b058d6-ec7d-4a53-8aef-5cf6b93fa449")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#FLG0160")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#FLG0160")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:PWR_FLAG")
		(at 49.53 55.88 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000006d")
		(property "Reference" "#FLG0161"
			(at 49.53 53.975 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "PWR_FLAG"
			(at 49.53 51.4858 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 49.53 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 49.53 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 49.53 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "c7efe309-e5f2-4c1c-88e5-e19c2c1f9f64")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#FLG0161")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#FLG0161")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 49.53 59.69 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000006e")
		(property "Reference" "#PWR0165"
			(at 49.53 66.04 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 49.657 64.0842 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 49.53 59.69 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 49.53 59.69 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 49.53 59.69 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "20b62e76-1a7c-42e7-a17d-0cc66252cda2")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0165")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0165")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:PWR_FLAG")
		(at 132.08 55.88 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000006f")
		(property "Reference" "#FLG0162"
			(at 132.08 53.975 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "PWR_FLAG"
			(at 132.08 51.4858 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 132.08 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 132.08 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 132.08 55.88 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "98c295c3-a32e-470f-81c7-f1ae73b46082")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#FLG0162")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#FLG0162")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+5V")
		(at 132.08 52.07 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000070")
		(property "Reference" "#PWR0166"
			(at 132.08 55.88 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+5V"
			(at 132.461 47.6758 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 132.08 52.07 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 132.08 52.07 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 132.08 52.07 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "541c9b36-b2b8-4045-84f8-42072f333fa3")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0166")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0166")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Regulator_Linear:AP2112K-3.3")
		(at 175.26 63.5 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000071")
		(property "Reference" "U4"
			(at 175.26 54.8132 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "AP2112K-3.3"
			(at 175.26 57.1246 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Package_TO_SOT_SMD:SOT-23-5"
			(at 175.26 55.245 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" "https://www.diodes.com/assets/Datasheets/AP2112.pdf"
			(at 175.26 63.5 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 175.26 63.5 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "616c6341-54b0-469e-ac1e-261919ebb36a")
		)
		(pin "2"
			(uuid "10fb5f2b-0fff-48fa-929f-d24471566baa")
		)
		(pin "3"
			(uuid "7b00c82e-5fae-43d4-bf3a-7386283fb533")
		)
		(pin "4"
			(uuid "312fee63-6dba-4cb5-aec5-32052808008d")
		)
		(pin "5"
			(uuid "360c9141-24dd-4a86-b628-96a01fff8a64")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "U4")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "U4")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+5V")
		(at 163.83 57.15 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000072")
		(property "Reference" "#PWR0170"
			(at 163.83 60.96 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+5V"
			(at 164.211 52.7558 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 163.83 57.15 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 163.83 57.15 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 163.83 57.15 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "3674f26b-4dbc-4326-935e-b047005089e0")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0170")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0170")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 186.69 57.15 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000073")
		(property "Reference" "#PWR0171"
			(at 186.69 60.96 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 187.071 52.7558 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 186.69 57.15 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 186.69 57.15 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 186.69 57.15 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "cea73965-3aca-406a-9956-ac5421d02839")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0171")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0171")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 163.83 68.58 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000074")
		(property "Reference" "C5"
			(at 166.751 67.4116 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10u"
			(at 166.751 69.723 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_SMD:C_0805_2012Metric"
			(at 164.7952 72.39 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 163.83 68.58 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 163.83 68.58 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "37aa344b-f944-4378-9494-69a96b1065be")
		)
		(pin "2"
			(uuid "022f5752-1b88-4971-8c3a-b8e6a69de704")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "C5")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "C5")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 186.69 68.58 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000075")
		(property "Reference" "C6"
			(at 189.611 67.4116 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10u"
			(at 189.611 69.723 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_SMD:C_0805_2012Metric"
			(at 187.6552 72.39 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 186.69 68.58 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 186.69 68.58 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "f9be027c-0009-472c-8cd3-2499c8d324f1")
		)
		(pin "2"
			(uuid "ac1086c0-7623-40fa-a6a5-918aca8e0ff6")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "C6")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "C6")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 175.26 76.2 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000076")
		(property "Reference" "#PWR0172"
			(at 175.26 82.55 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 175.387 80.5942 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 175.26 76.2 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 175.26 76.2 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 175.26 76.2 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "5c34e584-d84c-4eb2-99b5-4d3315560f4d")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0172")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0172")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 190.5 160.02 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000077")
		(property "Reference" "R8"
			(at 192.278 158.8516 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10k"
			(at 192.278 161.163 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0603_1608Metric"
			(at 188.722 160.02 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 190.5 160.02 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 190.5 160.02 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "7cbdac74-0d3c-441d-bbdf-26e8ba7a564a")
		)
		(pin "2"
			(uuid "68c5f893-5996-4d0b-bb01-e0801846d4dc")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R8")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R8")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 190.5 153.67 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000078")
		(property "Reference" "#PWR0173"
			(at 190.5 157.48 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 190.881 149.2758 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 190.5 153.67 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 190.5 153.67 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 190.5 153.67 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "18fe1b20-38a9-4694-bfb9-012776e0e308")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0173")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0173")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:C")
		(at 196.85 168.91 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000079")
		(property "Reference" "C7"
			(at 199.771 167.7416 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "1u"
			(at 199.771 170.053 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Capacitor_SMD:C_0603_1608Metric"
			(at 197.8152 172.72 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 196.85 168.91 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 196.85 168.91 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "9ca5d44d-59c9-4684-98ba-cedac7b06f28")
		)
		(pin "2"
			(uuid "d264004f-97c1-47c5-8257-52107b0a0103")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "C7")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "C7")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 196.85 172.72 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000007a")
		(property "Reference" "#PWR0174"
			(at 196.85 179.07 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 196.977 177.1142 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 196.85 172.72 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 196.85 172.72 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 196.85 172.72 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "b011e76e-1801-4622-b2d5-109d38dd875c")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0174")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0174")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 205.74 160.02 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000007b")
		(property "Reference" "R9"
			(at 207.518 158.8516 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "10k"
			(at 207.518 161.163 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0603_1608Metric"
			(at 203.962 160.02 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 205.74 160.02 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 205.74 160.02 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "c9a783c2-1f2f-4d56-90d9-08ef3d6f5a72")
		)
		(pin "2"
			(uuid "1273ecff-d1ef-4800-8762-741d8d3a4182")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R9")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R9")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 205.74 153.67 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000007c")
		(property "Reference" "#PWR0175"
			(at 205.74 157.48 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 206.121 149.2758 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 205.74 153.67 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 205.74 153.67 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 205.74 153.67 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "c7072d52-d9d3-49d8-ad7f-cbbe4b354855")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0175")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0175")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x06")
		(at 191.77 186.69 0)
		(mirror y)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000007d")
		(property "Reference" "J7"
			(at 193.8528 176.0982 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "UART_PROGRAM"
			(at 193.8528 178.4096 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Connector_PinHeader_1.27mm:PinHeader_1x06_P1.27mm_Vertical"
			(at 191.77 186.69 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 191.77 186.69 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 191.77 186.69 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "d47e0325-8f6a-46d0-95ed-6487af85b81e")
		)
		(pin "2"
			(uuid "c3d4b732-8bcc-4e1e-8cea-df5f8e38e273")
		)
		(pin "3"
			(uuid "ff4a839b-a79a-4751-bee4-de1b4b9b966c")
		)
		(pin "4"
			(uuid "6626f301-642e-4ad8-bf86-17be4f711b4a")
		)
		(pin "5"
			(uuid "9be36e2a-dc81-4597-8d9e-10aa91d38f32")
		)
		(pin "6"
			(uuid "0fb70907-9858-41f6-9631-f65a5e68705b")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J7")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J7")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 208.28 189.23 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000007e")
		(property "Reference" "#PWR0176"
			(at 208.28 193.04 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 208.661 184.8358 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 208.28 189.23 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 208.28 189.23 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 208.28 189.23 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "ecd7f62f-d2fd-4e38-b579-ab66fe7014e2")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0176")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0176")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 203.2 198.12 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-00006000007f")
		(property "Reference" "#PWR0177"
			(at 203.2 204.47 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 203.327 202.5142 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 203.2 198.12 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 203.2 198.12 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 203.2 198.12 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "2d5f42d8-628c-4b56-8ac2-608cb0dd2fb7")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0177")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0177")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Connector_Generic:Conn_01x06")
		(at 191.77 213.36 0)
		(mirror y)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000080")
		(property "Reference" "J8"
			(at 193.8528 202.7682 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "JTAG_DEBUG"
			(at 193.8528 205.0796 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Connector_PinHeader_1.27mm:PinHeader_1x06_P1.27mm_Vertical"
			(at 191.77 213.36 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 191.77 213.36 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 191.77 213.36 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "63d5fc2f-4ad8-4f48-ab4f-7fbd87f70805")
		)
		(pin "2"
			(uuid "583ff1bd-2c7a-45e2-8344-df6cb0b818be")
		)
		(pin "3"
			(uuid "4b0689bf-4643-49bd-ba22-e428cbcb0898")
		)
		(pin "4"
			(uuid "f5e7e69d-c8d4-47eb-8382-51feb0fe41f3")
		)
		(pin "5"
			(uuid "de4434af-fc77-4964-b53c-029cc982e268")
		)
		(pin "6"
			(uuid "32cac9ca-e3bf-47a3-9ca3-6c82527e1da6")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "J8")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "J8")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:+3V3")
		(at 208.28 215.9 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000081")
		(property "Reference" "#PWR0178"
			(at 208.28 219.71 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "+3V3"
			(at 208.661 211.5058 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 208.28 215.9 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 208.28 215.9 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 208.28 215.9 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "195621d9-f908-46bf-87df-2eecbdb0e2c2")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0178")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0178")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 203.2 224.79 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000082")
		(property "Reference" "#PWR0179"
			(at 203.2 231.14 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 203.327 229.1842 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 203.2 224.79 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 203.2 224.79 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 203.2 224.79 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "11ebf282-f139-4c71-885b-048add0e82af")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0179")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0179")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_SPST")
		(at 50.8 53.34 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000090")
		(property "Reference" "SW2"
			(at 50.8 47.371 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "CONTROLLER_POWER"
			(at 50.8 49.6824 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" "Button_Switch_SMD:SW_SPST_CK_RS282G05A3"
			(at 50.8 53.34 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 50.8 53.34 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 50.8 53.34 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "56b5c673-e869-47b4-b89c-36901261d1d4")
		)
		(pin "2"
			(uuid "88ac7ef6-45c3-4810-a321-4400ac5641a1")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "SW2")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "SW2")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Device:R")
		(at 57.15 59.69 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000091")
		(property "Reference" "R14"
			(at 58.928 58.5216 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Value" "100k"
			(at 58.928 60.833 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify left)
			)
		)
		(property "Footprint" "Resistor_SMD:R_0603_1608Metric"
			(at 55.372 59.69 90)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 57.15 59.69 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 57.15 59.69 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "795314cc-f42f-4476-8d10-165ef73259ac")
		)
		(pin "2"
			(uuid "4845b1b8-1e74-42ef-a0bd-548ec42e0757")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "R14")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "R14")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 57.15 64.77 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000092")
		(property "Reference" "#PWR0180"
			(at 57.15 71.12 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 57.277 69.1642 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 57.15 64.77 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 57.15 64.77 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 57.15 64.77 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "d8e19d1c-06b3-4f8d-a058-9ad8493da3e7")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0180")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0180")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "Switch:SW_Push")
		(at 181.61 168.91 90)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000093")
		(property "Reference" "SW1"
			(at 182.8292 167.7416 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Value" "RESET"
			(at 182.8292 170.053 90)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
				(justify right)
			)
		)
		(property "Footprint" "Button_Switch_SMD:SW_SPST_CK_KXT3"
			(at 176.53 168.91 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 176.53 168.91 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 181.61 168.91 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "79f39cb9-4c4e-4e13-abdb-7045cc3043bc")
		)
		(pin "2"
			(uuid "158c5768-95be-42f5-a44a-d8641e64a4e9")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "SW1")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "SW1")
					(unit 1)
				)
			)
		)
	)
	(symbol
		(lib_id "power:GND")
		(at 181.61 175.26 0)
		(unit 1)
		(body_style 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(in_pos_files yes)
		(dnp no)
		(uuid "00000000-0000-0000-0000-000060000094")
		(property "Reference" "#PWR0181"
			(at 181.61 181.61 0)
			(hide yes)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Value" "GND"
			(at 181.737 179.6542 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Footprint" ""
			(at 181.61 175.26 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Datasheet" ""
			(at 181.61 175.26 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(property "Description" ""
			(at 181.61 175.26 0)
			(show_name no)
			(do_not_autoplace no)
			(effects
				(font
					(size 1.27 1.27)
				)
			)
		)
		(pin "1"
			(uuid "33a93903-5397-4021-a176-c1375b0bbb57")
		)
		(instances
			(project "ESP32_FlightController"
				(path "/9c668f9a-03c5-4790-89cc-d267724e623e"
					(reference "#PWR0181")
					(unit 1)
				)
			)
			(project ""
				(path "/ee2f03c1-c37f-476e-8725-9667899dc27d"
					(reference "#PWR0181")
					(unit 1)
				)
			)
		)
	)
	(sheet_instances
		(path "/"
			(page "1")
		)
	)
	(embedded_fonts no)
)
