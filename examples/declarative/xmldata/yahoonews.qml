<Rect color="black" gradientColor="#AAAAAA" width="600" height="600">
  <resources>
        <XmlListModel id="feedModel" src="http://rss.news.yahoo.com/rss/oceania" query="doc($src)/rss/channel/item">
            <Role name="title" query="title/string()"/>
            <Role name="link" query="link/string()"/>
            <Role name="description" query="description/string()" isCData="true"/>
        </XmlListModel>
        <Component id="feedDelegate">
            <Item id="Delegate" height="{Wrapper.height + 10}">
                <MouseRegion anchors.fill="{Wrapper}" onPressed="Delegate.ListView.list.currentIndex = index;"
                             onClicked="if (Wrapper.state == 'Details') { Wrapper.state = '';} else {Wrapper.state = 'Details';}"/>
                <Rect id="Wrapper" y="5" height="{TitleText.height + 10}" width="580" color="#F0F0F0" radius="5">
                    <Text x="10" y="5" id="TitleText" text="{'&lt;a href=\'' + link + '\'&gt;' + title + '&lt;/a&gt;'}" font.bold="true" font.family="Helvetica" font.size="14" onLinkActivated="print('link clicked: ' + link)"/>
                    <Text x="10" id="Description" text="{description}" width="560" wrap="true" font.family="Helvetica"
                          anchors.top="{TitleText.bottom}" anchors.topMargin="5" opacity="0"/>

                    <states>
                        <State name="Details">
                            <SetProperty target="{Wrapper}" property="height" binding="contents.height + 10"/>
                            <SetProperty target="{Description}" property="opacity" value="1"/>
                        </State>
                    </states>
                    <transitions>
                        <Transition fromState="*" toState="Details" reversible="true">
                            <SequentialAnimation>
                                <NumericAnimation duration="200" properties="height" easing="easeOutQuad"/>
                                <NumericAnimation duration="200" properties="opacity"/>
                            </SequentialAnimation>
                        </Transition>
                    </transitions>
                </Rect>
            </Item>
        </Component>
    </resources>

    <ListView id="list" x="10" y="10" width="{parent.width - 20}" height="{parent.height - 20}" clip="true"
              model="{feedModel}" delegate="{feedDelegate}" currentItemPositioning="Snap"/>
</Rect>
