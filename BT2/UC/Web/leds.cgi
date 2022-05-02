t <html><head><title>AGP CONFIG</title>
t <script language=JavaScript>
t function AllSW(st) {
t  for(i=0;i<document.form1.length;i++) {
t   if(document.form1.elements[i].type=="checkbox"){
t    document.form1.elements[i].checked=st;
t   }
t  }
t  document.form1.submit();
t }
t </script></head>
i pg_header.inc
t <h2 align=center><br>Control AGP</h2>
t <form action=leds.cgi method=post name=form1>
t <input type=hidden value="led" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Setting</th></tr>
t <td><img src=pabb.gif>Seleccionar ganancia AGP:</td>
t <td><select name="ctrl" onchange="submit();">
c b c <option %s>1</option><option %s>5</option><option %s>10</option><option %s>50</option><option %s>100</option>
t </select></td></tr>
t <tr><td><img src=pabb.gif>Umbral Overload</TD>
c b d <td><input type=text name=umbral_OL maxlength= "3" value="%.3s"></td></tr>
t <td><img src=pabb.gif>Interrupcion por Overload:</td>
t <td><select name="ctrl2" onchange="submit();">
c b e <option %s>Activar</option><option %s>Desactivar</option></select></td></tr>
t </table>
t <input type=submit name=set value="Send" id="sbm"></form>
i pg_footer.inc
. End of script must be closed with period.


