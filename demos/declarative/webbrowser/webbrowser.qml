import "content"

Item {
    id: WebBrowser

    property var url : "http://www.qtsoftware.com"

    width: 640
    height: 480
    state: "Normal"

    Script {
        function zoomOut() {
            WebBrowser.state = "ZoomedOut";
        }
        function toggleZoom() {
            if(WebBrowser.state == "ZoomedOut") {
                Flick.centerX = WebView.mouseX;
                Flick.centerY = WebView.mouseY;
                WebBrowser.state = "Normal";
            } else {
                zoomOut();
            }
        }
    }

    Item {
        id: WebPanel
        anchors.fill: parent
        clip: true
        Rect {
            color: "#555555"
            anchors.fill: parent
        }
        Image {
            source: "content/pics/softshadow-bottom.png"
            width: WebPanel.width
            height: 16
        }
        Image {
            source: "content/pics/softshadow-top.png"
            width: WebPanel.width
            height: 16
            anchors.bottom: Footer.top
        }
        RectSoftShadow {
            x: -Flick.xPosition
            y: -Flick.yPosition
            width: WebView.width*WebView.scale
            height: Flick.y+WebView.height*WebView.scale
        }
        Item {
            id: HeaderSpace
            width: parent.width
            height: 60
            z: 1

            Image {
                id: Header
                source: "content/pics/header.png"
                width: parent.width
                height: 64
                state: "Normal"
                x: Flick.xPosition < 0 ? -Flick.xPosition : Flick.xPosition > Flick.viewportWidth-Flick.width
                                         ? -Flick.xPosition+Flick.viewportWidth-Flick.width : 0
                y: Flick.yPosition < 0 ? -Flick.yPosition : progressOff*
                                        (Flick.yPosition>height?-height:-Flick.yPosition)
                Text {
                    id: HeaderText

                    text: WebView.title!='' || WebView.progress == 1.0 ? WebView.title : 'Loading...'
                    elide: "ElideRight"

                    color: "white"
                    styleColor: "black"
                    style: "Raised"

                    font.family: "Helvetica"
                    font.size: 10
                    font.bold: true

                    anchors.left: Header.left
                    anchors.right: Header.right
                    anchors.leftMargin: 4
                    anchors.rightMargin: 4
                    anchors.top: Header.top
                    anchors.topMargin: 4
                    hAlign: "AlignHCenter"
                }
                Item {
                    width: parent.width
                    anchors.top: HeaderText.bottom
                    anchors.topMargin: 2
                    anchors.bottom: parent.bottom

                    Item {
                        id: UrlBox
                        height: 31
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.top: parent.top
                        clip: true
                        Image {
                            source: "content/pics/addressbar.sci"
                            anchors.fill: UrlBox
                        }
                        Image {
                            id: UrlBoxhl
                            source: "content/pics/addressbar-filled.sci"
                            width: parent.width*WebView.progress
                            height: parent.height
                            opacity: 1-Header.progressOff
                            clip: true
                        }
                        
                        /*
                        KeyProxy {
                            id: proxy
                            anchors.left: UrlBox.left
                            anchors.fill: UrlBox
                            focusable: true
                            targets: [keyActions,EditUrl]
                        }
                        KeyActions {
                            id: keyActions
                            return: "WebBrowser.url = EditUrl.text; proxy.focus=false;"
                        }
                        */
                        TextEdit {
                            id: EditUrl

                            text: WebView.url == '' ? ' ' : WebView.url
                            wrap: false
                            font.size: 11
                            color: "#555555"
                            focusOnPress: true

                            anchors.left: UrlBox.left
                            anchors.right: UrlBox.right
                            anchors.leftMargin: 6
                            anchors.verticalCenter: UrlBox.verticalCenter
                            anchors.verticalCenterOffset: 1
                        }
                    }
                }

                property real progressOff : 1
                states: [
                    State {
                        name: "Normal"
                        when: WebView.progress == 1.0
                        SetProperty {
                            target: Header
                            property: "progressOff"
                            value: 1
                        }
                    },
                    State {
                        name: "ProgressShown"
                        when: WebView.progress < 1.0
                        SetProperty {
                            target: Header
                            property: "progressOff"
                            value: 0
                        }
                    }
                ]
                transitions: [
                    Transition {
                        NumericAnimation {
                            target: Header
                            properties: "progressOff"
                            easing: "easeInOutQuad"
                            duration: 300
                        }
                    }
                ]
            }
        }
        Flickable {
            id: Flick
            width: parent.width
            viewportWidth: Math.max(parent.width,WebView.width*WebView.scale)
            viewportHeight: Math.max(parent.height,WebView.height*WebView.scale)
            anchors.top: HeaderSpace.bottom
            anchors.bottom: Footer.top
            anchors.left: parent.left
            anchors.right: parent.right

            property real centerX : 0
            property real centerY : 0

            WebView {
                id: WebView
                cacheSize: 4000000

                url: WebBrowser.url
                smooth: !Flick.moving
                focusable: true
                focus: true

                idealWidth: Flick.width
                idealHeight: Flick.height/scale
                scale: (width > 0) ? Flick.width/width*zoomedOut+(1-zoomedOut) : 1

                onUrlChanged: { Flick.xPosition=0; Flick.yPosition=0; zoomOut() }
                onDoubleClick: { toggleZoom() }

                property real zoomedOut : 1
            }
            Rect {
                id: WebViewTint
                color: "black"
                opacity: 0
                anchors.fill: WebView
                MouseRegion {
                    anchors.fill: WebViewTint
                    onClicked: { proxy.focus=false }
                }
            }
        }
        Image {
            id: Footer
            source: "content/pics/footer.sci"
            width: parent.width
            height: 43
            anchors.bottom: parent.bottom
            Rect {
                y: -1
                width: parent.width
                height: 1
                color: "#555555"
            }
            Item {
                id: backbutton
                width: back_e.width
                height: back_e.height
                anchors.right: reload.left
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                Image {
                    id: back_e
                    source: "content/pics/back.png"
                    anchors.fill: parent
                }
                Image {
                    id: back_d
                    source: "content/pics/back-disabled.png"
                    anchors.fill: parent
                }
                states: [
                    State {
                        name: "Enabled"
                        when: WebView.back.enabled==true
                        SetProperty {
                            target: back_e
                            property: "opacity"
                            value: 1
                        }
                        SetProperty {
                            target: back_d
                            property: "opacity"
                            value: 0
                        }
                    },
                    State {
                        name: "Disabled"
                        when: WebView.back.enabled==false
                        SetProperty {
                            target: back_e
                            property: "opacity"
                            value: 0
                        }
                        SetProperty {
                            target: back_d
                            property: "opacity"
                            value: 1
                        }
                    }
                ]
                transitions: [
                    Transition {
                        NumericAnimation {
                            properties: "opacity"
                            easing: "easeInOutQuad"
                            duration: 300
                        }
                    }
                ]
                MouseRegion {
                    anchors.fill: back_e
                    onClicked: { if (WebView.back.enabled) WebView.back.trigger() }
                }
            }
            Image {
                id: reload
                source: "content/pics/reload.png"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
            MouseRegion {
                anchors.fill: reload
                onClicked: { WebView.reload.trigger() }
            }
            Item {
                id: forwardbutton
                width: forward_e.width
                height: forward_e.height
                anchors.left: reload.right
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                Image {
                    id: forward_e
                    source: "content/pics/forward.png"
                    anchors.fill: parent
                    anchors.verticalCenter: parent.verticalCenter
                }
                Image {
                    id: forward_d
                    source: "content/pics/forward-disabled.png"
                    anchors.fill: parent
                }
                states: [
                    State {
                        name: "Enabled"
                        when: WebView.forward.enabled==true
                        SetProperty {
                            target: forward_e
                            property: "opacity"
                            value: 1
                        }
                        SetProperty {
                            target: forward_d
                            property: "opacity"
                            value: 0
                        }
                    },
                    State {
                        name: "Disabled"
                        when: WebView.forward.enabled==false
                        SetProperty {
                            target: forward_e
                            property: "opacity"
                            value: 0
                        }
                        SetProperty {
                            target: forward_d
                            property: "opacity"
                            value: 1
                        }
                    }
                ]
                transitions: [
                    Transition {
                        NumericAnimation {
                            properties: "opacity"
                            easing: "easeInOutQuad"
                            duration: 320
                        }
                    }
                ]
                MouseRegion {
                    anchors.fill: parent
                    onClicked: { if (WebView.forward.enabled) WebView.forward.trigger() }
                }
            }
        }
    }
    states: [
        State {
            name: "Normal"
            SetProperty {
                target: WebView
                property: "zoomedOut"
                value: 0
            }
            SetProperty {
                target: Flick
                property: "xPosition"
                value: Math.min(WebView.width-Flick.width,Math.max(0,Flick.centerX-Flick.width/2))
            }
            SetProperty {
                target: Flick
                property: "yPosition"
                value: Math.min(WebView.height-Flick.height,Math.max(0,Flick.centerY-Flick.height/2))
            }
        },
        State {
            name: "ZoomedOut"
            SetProperty {
                target: WebView
                property: "zoomedOut"
                value: 1
            }
        }
    ]
    transitions: [
        Transition {
            SequentialAnimation {
                SetPropertyAction {
                    target: WebView
                    property: "smooth"
                    value: false
                }
                ParallelAnimation {
                    NumericAnimation {
                        target: WebView
                        properties: "zoomedOut"
                        easing: "easeInOutQuad"
                        duration: 200
                    }
                    NumericAnimation {
                        target: Flick
                        properties: "xPosition,yPosition"
                        easing: "easeInOutQuad"
                        duration: 200
                    }
                }
                SetPropertyAction {
                    target: WebView
                    property: "smooth"
                    value: !Flick.moving
                }
            }
        }
    ]
}
