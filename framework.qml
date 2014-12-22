import QtQuick 2.0
//import QtMultimedia 5.0

Rectangle {
    // Default window size, can be resized by user or fullscreening
    width: 200
    height: 100
    color: "black"

    Item {
        id: framework
        objectName: "framework"
        width: parent.width
        height: parent.height
        property string currentScriptText
        property string renderScriptText
        property variant item
        onRenderScriptTextChanged: {
            //check if the script text really changed
            if (currentScriptText != renderScriptText)
            {
                //the script text changed, store new text
                currentScriptText = renderScriptText;
                //compile text to use it as an item
                var newItem;
                try {
                    //try compiling the item
                    newItem = Qt.createQmlObject("import QtQuick 2.0\n" + renderScriptText, framework, "renderScript");
                    //newItem = Qt.createQmlObject("import QtQuick 2.0\nRectangle { x: parent.width/2; y: parent.height/2; width: 1; height: 1; color: \"green\" }", framework, "renderScript");
                }
                catch (err) {
                    //an error occurred, show error marker
                    newItem = Qt.createQmlObject("import QtQuick 2.0\nRectangle { x: parent.width/2; y: parent.height/2; width: 1; height: 1; color: \"red\" }", framework, "renderScript");
                }
                if (newItem) {
                    //destroy old script and insert new one as item
                    newItem.objectName = "renderScript"
                    newItem.anchors.fill = framework;
                    if (item) {
                        item.destroy();
                    }
                    item = newItem;
                    //now notify C++ that out current render script changed
                    renderScriptChanged();
                }
            }
        } //onRenderScriptTextChanged
        signal renderScriptChanged()
    }
}
