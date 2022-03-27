t <html><head><title>Hora y fecha</title>
t <meta http-equiv="refresh" content="1"></head>
i pg_header.inc
t <h2 align=center><br>Hora y fecha</h2>
t <form action=time.cgi method=post name=cgi>
t <input type=hidden value="time" name=pg>
t <p><img src=pabb.gif>Hora y fecha</p>
c h <p><input type=text name=hora value="%2.2d:%2.2d:%2.2d   %2.2d/%2.2d/%4u"></p>
t </font></table>
t </form>
i pg_footer.inc
. End of script must be closed with period.
