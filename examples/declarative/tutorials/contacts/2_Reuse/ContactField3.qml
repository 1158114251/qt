<Item id="contactField"
    clip="true"
    width="230"
    height="30">
    <properties>
        <Property name="label" value="Name"/>
        <Property name="icon" value="../shared/pics/phone.png"/>
        <Property name="value"/>
    </properties>
    <RemoveButton3 id="removeButton"
        anchors.right="{parent.right}"
        anchors.top="{parent.top}" anchors.bottom="{parent.bottom}"
        expandedWidth="{contactField.width}"
        onConfirmed="print('Clear field text'); fieldText.text=''"/>
    <FieldText3 id="fieldText"
        width="{contactField.width-70}"
        anchors.right="{removeButton.left}" anchors.rightMargin="5"
        anchors.verticalCenter="{parent.verticalCenter}"
        label="{contactField.label}"
        text="{contactField.value}"/>
    <Image
        anchors.right="{fieldText.left}" anchors.rightMargin="5"
        anchors.verticalCenter="{parent.verticalCenter}"
        source="{contactField.icon}"/>
    <states>
        <State name="editingText" when="{fieldText.state == 'editing'}">
            <SetProperty target="{removeButton.anchors}" property="rightMargin" value="-35"/>
            <SetProperty target="{fieldText}" property="width" value="{contactField.width}"/>
        </State>
    </states>
    <transitions>
        <Transition fromState='' toState="*" reversible="true">
            <NumericAnimation properties="width,rightMargin" duration="200"/>
        </Transition>
    </transitions>
</Item>
