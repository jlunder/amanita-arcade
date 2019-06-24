EESchema Schematic File Version 4
LIBS:aa-input-controller-cache
EELAYER 26 0
EELAYER END
$Descr A2 23386 16535
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Mechanical:MountingHole_Pad H1
U 1 1 5CD715BA
P 3750 900
F 0 "H1" H 3850 905 50  0000 L CNN
F 1 "MountingHole_Pad" H 3850 860 50  0001 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5_Pad_Via" H 3750 900 50  0001 C CNN
F 3 "~" H 3750 900 50  0001 C CNN
	1    3750 900 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H2
U 1 1 5CD71A93
P 4250 900
F 0 "H2" H 4350 905 50  0000 L CNN
F 1 "MountingHole_Pad" H 4350 860 50  0001 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5_Pad_Via" H 4250 900 50  0001 C CNN
F 3 "~" H 4250 900 50  0001 C CNN
	1    4250 900 
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H3
U 1 1 5CE1AC04
P 3750 1700
F 0 "H3" H 3850 1705 50  0000 L CNN
F 1 "MountingHole_Pad" H 3850 1660 50  0001 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5_Pad_Via" H 3750 1700 50  0001 C CNN
F 3 "~" H 3750 1700 50  0001 C CNN
	1    3750 1700
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole_Pad H4
U 1 1 5CE1ADFE
P 4200 1700
F 0 "H4" H 4300 1705 50  0000 L CNN
F 1 "MountingHole_Pad" H 4300 1660 50  0001 L CNN
F 2 "MountingHole:MountingHole_2.7mm_M2.5_Pad_Via" H 4200 1700 50  0001 C CNN
F 3 "~" H 4200 1700 50  0001 C CNN
	1    4200 1700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3750 1000 3750 1100
Wire Wire Line
	4200 1900 4200 1800
Wire Wire Line
	3750 1800 3750 1900
Wire Wire Line
	4250 1000 4250 1100
$Comp
L power:GND #PWR0101
U 1 1 5D7EA0AB
P 4000 1200
F 0 "#PWR0101" H 4000 950 50  0001 C CNN
F 1 "GND" H 4005 1027 50  0001 C CNN
F 2 "" H 4000 1200 50  0001 C CNN
F 3 "" H 4000 1200 50  0001 C CNN
	1    4000 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3750 1100 4000 1100
Wire Wire Line
	4000 1100 4000 1200
Connection ~ 4000 1100
Wire Wire Line
	4000 1100 4250 1100
$Comp
L power:GND #PWR0102
U 1 1 5D0E7BF1
P 4000 2000
F 0 "#PWR0102" H 4000 1750 50  0001 C CNN
F 1 "GND" H 4005 1827 50  0001 C CNN
F 2 "" H 4000 2000 50  0001 C CNN
F 3 "" H 4000 2000 50  0001 C CNN
	1    4000 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 1900 4000 2000
Connection ~ 4000 1900
Wire Wire Line
	4000 1900 4200 1900
Wire Wire Line
	3750 1900 4000 1900
$Comp
L aa-shared:Fiducial FD5
U 1 1 5D0E7DDE
P 3300 1950
F 0 "FD5" H 3428 1950 50  0000 L CNN
F 1 "Fiducial" H 3300 2100 50  0001 C CNN
F 2 "aa-shared:StencilAlignment_1.27mm" H 3300 1950 50  0001 C CNN
F 3 "" H 3300 1950 50  0001 C CNN
	1    3300 1950
	1    0    0    -1  
$EndComp
$Comp
L aa-shared:Fiducial FD1
U 1 1 5D0E7E00
P 3300 1650
F 0 "FD1" H 3428 1650 50  0000 L CNN
F 1 "Fiducial" H 3300 1800 50  0001 C CNN
F 2 "Fiducial:Fiducial_1mm_Dia_2.54mm_Outer_CopperTop" H 3300 1650 50  0001 C CNN
F 3 "" H 3300 1650 50  0001 C CNN
	1    3300 1650
	1    0    0    -1  
$EndComp
$Comp
L aa-shared:Fiducial FD6
U 1 1 5D0E80B6
P 4650 1150
F 0 "FD6" H 4778 1150 50  0000 L CNN
F 1 "Fiducial" H 4650 1300 50  0001 C CNN
F 2 "aa-shared:StencilAlignment_1.27mm" H 4650 1150 50  0001 C CNN
F 3 "" H 4650 1150 50  0001 C CNN
	1    4650 1150
	1    0    0    -1  
$EndComp
$Comp
L aa-shared:Fiducial FD2
U 1 1 5D0E80BD
P 4650 850
F 0 "FD2" H 4778 850 50  0000 L CNN
F 1 "Fiducial" H 4650 1000 50  0001 C CNN
F 2 "Fiducial:Fiducial_1mm_Dia_2.54mm_Outer_CopperTop" H 4650 850 50  0001 C CNN
F 3 "" H 4650 850 50  0001 C CNN
	1    4650 850 
	1    0    0    -1  
$EndComp
$EndSCHEMATC
