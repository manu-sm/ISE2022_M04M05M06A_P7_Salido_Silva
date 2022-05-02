t <html><head><title>Configl</title></head>
i pg_header.inc
t <h2 align=center><br>Configl</h2>
t <form action=config.cgi method=post name=cgi>
t <input type=hidden value="lcd" name=pg>
t <table border=0 width=99%><font size="3">
t <tr bgcolor=#aaccff>
t  <th width=40%>Item</th>
t  <th width=60%>Setting</th></tr>
# Here begin data setting which is formatted in HTTP_CGI.C module
t <tr><td><img src=pabb.gif>Hora</td>
c i 1 <td><input type=text name=hora size=20 maxlength=20 value="%s"></td></tr>
t <tr><td><img src=pabb.gif>Fecha</TD>
c i 2 <td><input type=text name=fecha size=20 maxlength=20 value="%s"></td></tr>
t <tr><td><img src=pabb.gif>Umbral ADC</TD>
c i 3 <td><input type=text name=umbral_adc size=3 maxlength=3 value="%d"></td></tr>
t <tr><td><img src=pabb.gif>Seleccion SNTP 1</TD>
c i 4 <td><input type=checkbox name=sntp_1 OnClick="submit();" %s>SNTP: Rediris 130.266.0.1 (Madrid)</td></tr>
t <tr><td><img src=pabb.gif>Seleccion SNTP 2</TD>
c i 5 <td><input type=checkbox name=sntp_2 OnClick="submit();" %s>SNTP: IMDEA 193.147.107.33 (Pozuelo)</td></tr>
#t <tr><td><img src=pabb.gif>Borrar Flash.</TD>
#c i 6 <td><input type=checkbox name=erase_flash OnClick="submit();" %s>Borrar Flash (Sector 18)</td></tr>
#t <tr><td><img src=pabb.gif>Fecha</TD>
#c i	7 <td>%s</td></tr>
#t <tr><td><img src=pabb.gif>Sector 18</td>
#c i 7 <td><input type=textarea name=flash_read size=192 maxlength=192 value="%s" readonly></td></tr>
t </font></table>
# Here begin button definitions
t <p align=center>
t <input type=submit name=set value="Send" id="sbm">
t <input type=reset value="Undo">
t </p></form>
i pg_footer.inc
. End of script must be closed with period.
