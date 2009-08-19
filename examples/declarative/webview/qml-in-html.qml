import Qt 4.6

// The WebView supports QML data through the HTML OBJECT tag
Rectangle {
    color:"blue"
    Flickable {
        width: parent.width
        height: parent.height/2
        viewportWidth: Web.width*Web.scale
        viewportHeight: Web.height*Web.scale
        WebView {
            id: Web
            width: 250
            height: 420
            scale: 0.75
            smooth: true
            settings.pluginsEnabled: true
            html: "<html>\
                <body bgcolor=white>\
                    These are QML plugins, shown in a QML WebView via HTML OBJECT tags, all scaled to 75%\
                    and placed in a Flickable area...\
                    <table border=1>\
                        <tr><th>Duration <th>Color <th>Plugin\
                        <tr><td>500      <td>red   <td><OBJECT data=content/SpinSquare.qml TYPE=application/x-qt-plugin width=100 height=100 period=500 color=red />\
                        <tr><td>2000     <td>blue  <td><OBJECT data=content/SpinSquare.qml TYPE=application/x-qt-plugin width=100 height=100 period=2000 color=blue />\
                        <tr><td>1000     <td>green <td><OBJECT data=content/SpinSquare.qml TYPE=application/x-qt-plugin width=100 height=100 period=1000 color=green />\
                    </table>\
                </body>\
                </html>"
        }
    }
}
