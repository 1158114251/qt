import Qt 4.6

QtObject {
   property var other
   other: Alias3 { id: MyAliasObject }

   property int value: MyAliasObject.obj.myValue
}

