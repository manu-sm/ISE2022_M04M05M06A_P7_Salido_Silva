t <html><head><title>Estado Overload</title>
t <meta http-equiv="refresh" content="1"></head>
i pg_header.inc
t <h2 align=center><br>Estado Overload</h2>
t <form action=overload.cgi method=post name=cgi>
t <input type=hidden value="overload" name=pg>
# Here begin data setting which is formatted in HTTP_CGI.C module
c l <p> <img src=pabb.gif>Estado Overload: <input type=text name=estado_ol size=20 maxlength=5 value="%s"></p>
t </form>
i pg_footer.inc
. End of script must be closed with period.
