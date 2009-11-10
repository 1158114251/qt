import Qt 4.6

Object {
    property bool xmlTest: false
    property bool dataOK: false

    Script {
        function checkElement(e)
        {
            if (e.tagName != "root")
                return;

            if (e.nodeName != "root")
                return;

            if (e.nodeValue != null)
                return;

            if (e.nodeType != 1)
                return;

            var childTagNames = [ "person", "fruit" ];

            if (e.childNodes.length != childTagNames.length)
                return;

            for (var ii = 0; ii < childTagNames.length; ++ii) {
                if (e.childNodes[ii].tagName != childTagNames[ii])
                    return;
            }

            if (e.childNodes[childTagNames.length + 1] != null)
                return;

            if (e.firstChild.tagName != e.childNodes[0].tagName)
                return;

            if (e.lastChild.tagName != e.childNodes[1].tagName)
                return;

            if (e.previousSibling != null)
                return;

            if (e.nextSibling != null)
                return;

            if (e.attributes == null)
                return;

            var attr1 = e.attributes["attr"];
            if (attr1.nodeValue != "value")
                return;

            var attrIdx = e.attributes[0];
            if (attrIdx.nodeValue != "value")
                return;

            var attr2 = e.attributes["attr2"];
            if (attr2.nodeValue != "value2")
                return;

            var attr3 = e.attributes["attr3"];
            if (attr3 != null)
                return;

            var attrIdx2 = e.attributes[11];
            if (attrIdx2 != null)
                return;

            xmlTest = true;
        }

        function checkXML(document)
        {
            checkElement(document.documentElement);
        }
    }

    Component.onCompleted: {
        var x = new XMLHttpRequest;

        x.open("GET", "element.xml");

        // Test to the end
        x.onreadystatechange = function() {
            if (x.readyState == XMLHttpRequest.DONE) {

                dataOK = true;

                if (x.responseXML != null)
                    checkXML(x.responseXML);

            }
        }

        x.send()
    }
}



