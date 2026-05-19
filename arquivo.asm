sECTION   TEXT
INPUT    		OLD_DATA
LOAD OLD_DATA		
L1: DIV     		 DOIS
STORE        NEW_DATA                   ; ; ; ;; 
MULT DOIS;;; 
STORE TMP_DATA;;;                    
LoAD oLD_DaTA        
;w
SUB TMP_DATA;132

STORE TMP_DATA



OUTPUT TMP_DATA

COPY NEW_DATA, OLD_DATA

LOAD OLD_DATA
 JMPP L1
STOP

SECTION DATA                            ; esse programa imprime o valor de entrada em binario a partir do bit menos significativo.
DOIS: CONST 2
OLD_DATA:

SPACE
NEW_DATA: SPACE
             TMP_DATA: SPACE