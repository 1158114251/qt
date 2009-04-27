<Rect id="rect" color="blue" width="40" height="30">
    <Rect id="dot" color="red" width="3" height="3" x="{rect.width/2}" y="{rect.height/2}"/>
    <MouseRegion id="mr" anchors.fill="{rect}"/>
    <Connection sender="{mr}" signal="clicked(mouse)">
        color="green";
        dot.x = mouse.x-1;
        dot.y = mouse.y-1;
    </Connection>
</Rect>
