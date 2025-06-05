PROJ_NAME = main.exe
C_FILES = src/*.c
INCLUDE_PATH = libs
RAYLIB_FLAGS = -Llibs -lraylib -lopengl32 -lgdi32 -lwinmm

default:
	gcc -Wall -Wextra -std=c99 -I$(INCLUDE_PATH) $(C_FILES) $(RAYLIB_FLAGS) -o $(PROJ_NAME)