ondevice:
	g++ -D ONDEV -lm -lpcap -lrt -lpthread -lstdc++ mro.c glp.c heur.c gb.c radio.c predictor.c lora/*.o -o mro
simu:
	g++ -lm -lpcap -lrt -lpthread -lstdc++ mro.c glp.c heur.c gb.c radio.c predictor.c lora/*.o -o mro
bigcover:
	g++ -D BIGCOVER -lm -lpcap -lrt -lpthread -lstdc++ mro.c glp.c heur.c gb.c radio.c predictor.c lora/*.o -o mro
random:
	g++ -D RANDOM -lm -lpcap -lrt -lpthread -lstdc++ mro.c glp.c heur.c gb.c radio.c predictor.c lora/*.o -o mro