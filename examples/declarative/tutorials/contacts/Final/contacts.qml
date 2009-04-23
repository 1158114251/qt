<Rect id="contacts"
    width="240"
    height="320"
    color="black">
    <properties>
        <Property name="mode" value="list"/>
        <Property name="mouseGrabbed" value="false"/>
    </properties>
    <resources>
        <SqlConnection id="contactDatabase" name="qmlConnection" driver="QSQLITE" databaseName="../shared/contacts.sqlite"/>
        <SqlQuery id="contactList" connection="{contactDatabase}">
            <query>SELECT recid AS contactid, label, email, phone FROM contacts WHERE lower(label) LIKE lower(:searchTerm) ORDER BY label, recid</query>
            <bindings>
                <SqlBind name=":searchTerm" value="{searchBar.text + '%' }"/>
            </bindings>
        </SqlQuery>
        <Component id="contactDelegate">
            <Item id="wrapper"
                x="0"
                width="{ListView.view.width}"
                height="34">
                <Text id="label"
                    x="40" y="12"
                    width="{parent.width-30}" 
                    text="{model.label}"
                    color="white"
                    font.bold="true">
                    <children>
                        <MouseRegion
                            anchors.fill="{parent}">
                            <onClicked>
                                Details.qml = 'Contact.qml';
                                wrapper.state='opened';
                                contacts.mode = 'edit';
                            </onClicked>
                        </MouseRegion>
                    </children>
                </Text>
                <Item id="Details"
                    anchors.fill="{wrapper}"
                    opacity="0">
                    <Bind target="{Details.qmlItem}" property="contactid" value="{model.contactid}"/>
                    <Bind target="{Details.qmlItem}" property="label" value="{model.label}"/>
                    <Bind target="{Details.qmlItem}" property="phone" value="{model.phone}"/>
                    <Bind target="{Details.qmlItem}" property="email" value="{model.email}"/>
                </Item>
                <states>
                    <State name='opened'>
                        <SetProperty target="{wrapper}" property="height" value="{contactListView.height}"/>
                        <SetProperty target="{contactListView}" property="yPosition" value="{wrapper.y}"/>
                        <SetProperty target="{contactListView}" property="locked" value="1"/>
                        <SetProperty target="{label}" property="opacity" value="0"/>
                        <SetProperty target="{Details}" property="opacity" value="1"/>
                    </State>
                </states>
                <transitions>
                    <Transition>
                        <NumericAnimation duration="500" properties="yPosition,height,opacity"/>
                    </Transition>
                </transitions>
                <Connection sender="{cancelEditButton}" signal="clicked()">
                    if (wrapper.state == 'opened' &amp;&amp; !contacts.mouseGrabbed) {
                        wrapper.state = '';
                        contacts.mode = 'list';
                    }
                </Connection>
                <Connection sender="{confirmEditButton}" signal="clicked()">
                    if (wrapper.state == 'opened' &amp;&amp; !contacts.mouseGrabbed) {
                        print('confirm and close edit');
                        Details.qmlItem.update.emit();
                        wrapper.state = '';
                        contacts.mode = 'list';
                        contactList.exec();
                    }
                </Connection>
            </Item>
        </Component>
    </resources>
    <Button id="newContactButton"
        anchors.top="{parent.top}" anchors.topMargin="5"
        anchors.right="{parent.right}" anchors.rightMargin="5"
        icon="../shared/pics/new.png"
        onClicked="newContactItem.label = ''; newContactItem.phone = ''; newContactItem.email = ''; contacts.mode = 'new'"
        opacity="{contacts.mode == 'list' ? 1 : 0}"/>
    <Button id="confirmEditButton"
        anchors.top="{parent.top}" anchors.topMargin="5"
        anchors.left="{parent.left}" anchors.leftMargin="5"
        icon="../shared/pics/ok.png"
        opacity="{contacts.mode == 'list' || contacts.mouseGrabbed ? 0 : 1}"/>
    <Button id="cancelEditButton"
        anchors.top="{parent.top}" anchors.topMargin="5"
        anchors.right="{parent.right}" anchors.rightMargin="5"
        icon="../shared/pics/cancel.png"
        opacity="{contacts.mode == 'list' || contacts.mouseGrabbed ? 0 : 1}"/>
    <ListView id="contactListView"
        anchors.left="{parent.left}"
        anchors.right="{parent.right}"
        anchors.top="{cancelEditButton.bottom}"
        anchors.bottom="{searchBarWrapper.bottom}"
        clip="true"
        model="{contactList}"
        delegate="{contactDelegate}"
        focus="{contacts.mode != 'list'}"/>
    <Contact id="newContactItem"
        anchors.fill="{contactListView}"
        opacity="0"/>
    <Connection sender="{confirmEditButton}" signal="clicked()">
        if (contacts.mode == 'new' &amp;&amp; contacts.mouseGrabbed != 'true') {
            newContactItem.insert.emit();
            contacts.mode = 'list';
            contactList.exec();
        }
    </Connection>
    <Connection sender="{cancelEditButton}" signal="clicked()">
        if (contacts.mode == 'new' &amp;&amp; contacts.mouseGrabbed != 'true') {
            contacts.mode = 'list';
        }
    </Connection>
    <FocusRealm id="searchBarWrapper"
        height="30"
        anchors.bottom="{parent.bottom}"
        anchors.left="{parent.left}" anchors.right="{parent.right}"
        anchors.bottomMargin="0"
        focus="{contacts.mode == 'list'}">
        <SearchBar id="searchBar" anchors.fill="{parent}"/>
        <states>
            <State name="searchHidden" when="{searchBar.text == '' || contacts.mode != 'list'}">
                <SetProperty target="{searchBarWrapper.anchors}" property="bottomMargin" value="-30"/>
            </State>
        </states>
        <transitions>
            <Transition fromState="*" toState="*">
                <NumericAnimation property="bottomMargin" duration="250"/>
            </Transition>
        </transitions>
    </FocusRealm>
    <states>
        <State name="editNewState" when="{contacts.mode == 'new'}">
            <SetProperty target="{contactListView}" property="opacity" value="0"/>
            <SetProperty target="{newContactItem}" property="opacity" value="1"/>
        </State>
    </states>
    <transitions>
        <Transition fromState="*" toState="*">
            <NumericAnimation property="opacity" duration="500"/>
        </Transition>
    </transitions>
</Rect>
