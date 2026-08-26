@echo off
set EXEPATH=0_Mamgma_Test.exe
if not exist %EXEPATH% set EXEPATH=..\..\..\Debug\0_Mamgma_Test.exe
if not exist %EXEPATH% set EXEPATH=..\..\..\x64\Release\0_Mamgma_Test.exe
if not exist %EXEPATH% set EXEPATH=..\..\..\Release\0_Mamgma_Test.exe
if not exist %EXEPATH% set EXEPATH=..\..\..\x64\Debug\0_Mamgma_Test.exe

echo =======================================================
echo  AUTOMATED CLI TEST BATCH SUITE
echo =======================================================

echo [TEST 1] Slicing Cube with Plane Y=0 (OBJ -> OBJ)...
%EXEPATH% Data\01_Cube.obj Data\Plane_Y_Zero.xml out_cube_piece_

echo [TEST 2] Slicing Non-Convex Sine Wave into Multiple Islands...
%EXEPATH% Data\03_SineWave_NonConvex.obj Data\Plane_Wave_Cutter.xml out_wave_island_

echo [TEST 3] Diagonal 45-degree cut on Sphere...
%EXEPATH% Data\02_Sphere.model.xml Data\Plane_Diagonal_45Deg.xml out_sphere_diag_

echo [TEST 4] Plane Miss Test (No intersection)...
%EXEPATH% Data\01_Cube.obj Data\Plane_Miss.xml out_cube_miss_

echo [TEST 5] Plain text plane format test...
%EXEPATH% Data\01_Cube.obj Data\Plane_Simple_Text.txt out_cube_txt_plane_

echo =======================================================
echo  ALL BATCH TESTS FINISHED!
echo =======================================================
pause
