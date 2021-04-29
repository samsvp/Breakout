g++ -std=c++11 -o breakout \
               src/main.cpp src/glad.c src/game.cpp src/stb_image.cpp \
               src/shader.cpp src/texture.cpp src/resource_manager.cpp \
               src/sprite_renderer.cpp src/game_object.cpp src/game_level.cpp \
               src/ball_object.cpp src/particle_generator.cpp src/post_processor.cpp \
               src/text_renderer.cpp \
             -I/usr/include/freetype2 -I/usr/include/libpng16 -lGL -lGLU -lglfw -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lsfml-audio -lfreetype

