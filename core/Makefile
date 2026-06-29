all:
\tgcc -Wall -Wextra -O2 -Iinclude -c state_machine.c -o state_machine.o
\tgcc -Wall -Wextra -O2 -Iinclude -c process_manager.c -o process_manager.o
\tgcc -Wall -Wextra -O2 -Iinclude -c gesture_engine.c -o gesture_engine.o
\tgcc -Wall -Wextra -O2 -Iinclude -c memory.c -o memory.o
\tgcc -Wall -Wextra -O2 -Iinclude -c runtime_loader.c -o runtime_loader.o
\tgcc -Wall -Wextra -O2 -Iinclude -c api_translator.c -o api_translator.o
\tar rcs libomnios_core.a *.o
\techo "Build complete!"

clean:
\trm -f *.o libomnios_core.a

