import Qt.test 1.0

MyQmlObject{
    id: obj
    objectName: "obj"
    function createOne()
    {
        obj.objectProperty = createQmlObject('import Qt.test 1.0; MyQmlObject{objectName:"emptyObject"}', obj);
    }

    function createTwo()
    {
        var component = createComponent('dynamicCreation.helper.qml');
        obj.objectProperty = component.createObject();
    }
}
