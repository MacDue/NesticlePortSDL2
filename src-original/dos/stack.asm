                .386
        .model flat
        
        .data
    
        ALIGN DWORD        
stack     dd   4000h DUP (?)
stackend  label DWORD        

oldss   dw ?
oldsp   dd ?

        
        .code
        
        
        PUBLIC SwitchStack_,SwitchBack_
        
       
        
SwitchStack_ PROC NEAR
        pop ebx  ;get calling funk
        mov [oldsp],esp
        mov [oldss],ss
        
        mov ax,ds
        mov ss,ax
        mov esp,OFFSET stackend-4
        
        jmp ebx ;return
SwitchStack_ ENDP
        
        
SwitchBack_ PROC NEAR
        pop ebx
        mov ss,[oldss]
        mov esp,[oldsp]
        jmp ebx ;return
SwitchBack_ ENDP                
                       
        

        END        
   
