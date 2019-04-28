        .386
        .model flat
        LOCALS

        PUBLIC _initmouse,_readmouse,_setmouseextent

        .code

       
        
_initmouse PROC 
        push edi
        mov eax,0
        int 33h
             
        mov eax,4
        mov ecx,160
        mov edx,100
        int 33h   

;        mov eax,0Fh
;        mov ecx,4
;        mov edx,16
;        int 33h   

        pop edi
        ret
_initmouse ENDP


_setmouseextent PROC
        ARG XW:DWORD,YW:DWORD
        push ebp
        mov ebp,esp
        pusha

        mov eax,7
        mov ecx,0
        mov edx,XW
        shl edx,1
        int 33h   

        mov eax,8
        mov ecx,0
        mov edx,YW
        int 33h   

        popa
        pop ebp
        ret
_setmouseextent ENDP


_readmouse PROC
        ARG X:DWORD,Y:DWORD
        push ebp
        mov ebp,esp
        push ebx

        mov eax,03h
        int 33h
        
        mov eax,X        
        and ecx,0FFFFh
        shr ecx,1
        mov [eax],ecx

        mov eax,Y        
        and edx,0FFFFh
        mov [eax],edx
        
        mov eax,ebx ;;buttons
        and eax,0FFh
       
        pop ebx
        pop ebp
        ret
_readmouse ENDP
        END

