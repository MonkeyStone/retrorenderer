objects = build/Geometry.o build/Image.o build/Mesh.o build/Render.o build/Test.o
flags = -g -Wall

test: RetroRenderer
	./RetroRenderer models/wizard/wizard.obj 32 32 0.45 -o render.tga --pitch -30 --yaw 315 --cull front
	# open render.tga

RetroRenderer: $(objects)
	g++ -o RetroRenderer $+ $(flags)

$(objects): build/%.o: src/%.cpp
	g++ -c -o $@ $< $(flags)
