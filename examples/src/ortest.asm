   MOV   A, #10
   MOV   B, #12
   MOV   C, #10
   MOV   E, #15

   CEQ   A, B
OR CEQ   A, C
OR CGT   A, E
   ADD.P H, #1

   ERR
