t <html><head><title>Registros Flash</title></head>
i pg_header.inc
t <h2 align=center><br>Registros Memoria Flash</h2>
t <p><font size="3">Para observar los registros de la memoria introducir el
t  numero de registro deseado <b>(1-127)</b> y enviar, aparecerá escrito el contenido
t  en valor hexadecimal en la linea inferior. El primer byte indica el evento que
t  fue registrado, los 7 restantes el timestamp del momento en el que sucedió.</font></p>
t <p><font size="2">Timestamp: Byte nº -> 2: Hora  3: Minuto  4: Segundo  5: Dia  6: Mes
t  7 (MSByte) y 8 (LSByte): Año</font></p>
t <p><font size="2">Eventos:</font></p>
t <p><font size="2">- Cambio de ganancia "0X" X=> 0:x1 , 1:x5 , 2:x10 , 3:x50 , 4:x100</font></p>
t <p><font size="2">- Evento Overload --> (Valor hexadecimal - 100)/10=
t  Umbral Overload cuando se registra la interrupcion. (V)</font></p>
t <form action=flash.cgi method=post name=cgi>
t <input type=hidden value="flash" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Setting</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t <tr><td><img src=pabb.gif>Seleccione Registro [1-127]</td>
c d 1 <td><input type=text name=num_reg size=3 maxlength=3 value="%d"></td></tr>
t <tr><td><img src=pabb.gif>Sector 19</td>
c d 2 <td><input type=textarea name=reg_leido size=24 maxlength=24 value="%s" readonly></td></tr>
t </font></table>
# Here begin button definitions
t <p align=center>
t <input type=submit name=set value="Send" id="sbm">
t <input type=reset value="Undo">
t </p></form>
i pg_footer.inc
. End of script must be closed with period.

