##### INSTRUCOES #####
Para executar os testes via terminal: 

altere o frame a ser lido pelo decoder alterando a atribui��o do vetor de ints:
[Linha 60] int can_bit = standard_frame[i]; 


execute no terminal, no diret�rio do can-controller, os comandos:

-	g++ test-bench.cpp Decoder.cpp Encoder.cpp BitDeStuffing.cpp Crc.cpp
-	a.exe 
	ou 
-	./a.out

As informa��es de decodifica��o e encodifica��o ser�o escritas no console