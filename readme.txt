##### INSTRUCOES #####
Para executar os testes via terminal: 

altere o frame a ser lido pelo decoder alterando a atribuição do vetor de ints:
[Linha 60] int can_bit = standard_frame[i]; 


execute no terminal, no diretório do can-controller, os comandos:

-	g++ test-bench.cpp Decoder.cpp Encoder.cpp BitDeStuffing.cpp Crc.cpp
-	a.exe 
	ou 
-	./a.out

As informações de decodificação e encodificação serão escritas no console