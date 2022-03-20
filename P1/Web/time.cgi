t <html><head><title>Hora y fecha</title>
t <meta http-equiv="refresh" content="1"></head>
i pg_header.inc
t <h2 align=center><br>Hora y fecha</h2>
t <form action=time.cgi method=post name=cgi>
t <input type=hidden value="time" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Setting</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t <tr><td><img src=pabb.gif>Hora</td>
c h 1 <td><input type=text name=hora value="%2.2d:%2.2d:%2.2d"></td></tr>
t <tr><td><img src=pabb.gif>Fecha</TD>
c h	2 <td><input type=text name=fecha value="%2.2d/%2.2d/%4u"></td></tr>
c b 0 <td><input type=checkbox name=update OnClick="submit();" %s>Actualizar hora</td>
t </font></table>
#t <p><img src=pabb.gif>Hora y fecha</p>
#c h <p><input type=text name=hora value="%2.2d:%2.2d:%2.2d   %2.2d/%2.2d/%4u"></p>
#t </font></table>
t </form>
i pg_footer.inc
. End of script must be closed with period.
