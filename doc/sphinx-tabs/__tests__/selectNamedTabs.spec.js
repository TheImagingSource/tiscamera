const { selectNamedTabs } = require("../sphinx_tabs/static/tabs");

describe("selectNamedTabs", () => {
    it("selects tabs with name, but not clickedId", () => {
        document.body.innerHTML =
        '<div>' +
        '  <div>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-1-1" aria-selected="true">Test tab 1</button>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-1-2" aria-selected="false" name="sharedName" id="clicked">Test tab 2</button>' +
        '  </div>' +
        '  <div id="panel-1-1">Test panel 1</div>' +
        '  <div id="panel-1-2" hidden="true"">Test panel 2</div>' +
        '</div>' +
        '<div>' +
        '  <div>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-2-1" aria-selected="true">Test tab 1</button>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-2-2" aria-selected="false" name="sharedName" id="notClicked">Test tab 2</button>' +
        '  </div>' +
        '  <div id="panel-2-1">Test panel 1</div>' +
        '  <div id="panel-2-2" hidden="true">Test panel 2</div>' +
        '</div>';

        let clickedTab = document.getElementById('clicked');
        let namedTab = document.getElementById('notClicked');
        let clickedTabPanel = document.getElementById('panel-1-2');
        let namedTabPanel = document.getElementById('panel-2-2');
        expect(clickedTab.getAttribute('aria-selected')).toEqual('false');
        expect(namedTab.getAttribute('aria-selected')).toEqual('false');
        expect(clickedTabPanel.getAttribute('hidden')).toEqual('true');
        expect(namedTabPanel.getAttribute('hidden')).toEqual('true');

        selectNamedTabs("sharedName", clickedId="clicked");

        expect(clickedTab.getAttribute('aria-selected')).toEqual('false');
        expect(namedTab.getAttribute('aria-selected')).toEqual('true');
        expect(clickedTabPanel.getAttribute('hidden')).toEqual("true");
        expect(namedTabPanel.getAttribute('hidden')).toEqual(null);
    })
    it("selects first tab with same name in another tabList", () => {
        document.body.innerHTML =
        '<div>' +
        '  <div>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-1-1" aria-selected="true">Test tab 1</button>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-1-2" aria-selected="false" name="sharedName" id="clicked">Test tab 2</button>' +
        '  </div>' +
        '  <div id="panel-1-1">Test panel 1</div>' +
        '  <div id="panel-1-2" hidden="true"">Test panel 2</div>' +
        '</div>' +
        '<div>' +
        '  <div>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-2-1" aria-selected="true">Test tab 1</button>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-2-2" aria-selected="false" name="sharedName" id="notClicked1">Test tab 2</button>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-2-3" aria-selected="false" name="sharedName" id="notClicked2">Test tab 3</button>' +
        '  </div>' +
        '  <div id="panel-2-1">Test panel 1</div>' +
        '  <div id="panel-2-2" hidden="true">Test panel 2</div>' +
        '  <div id="panel-2-3" hidden="true">Test panel 3</div>' +
        '</div>';

        let clickedTab = document.getElementById('clicked');
        let firstNamedTab = document.getElementById('notClicked1');
        let secondNamedTab = document.getElementById('notClicked2');

        expect(clickedTab.getAttribute('aria-selected')).toEqual('false');
        expect(firstNamedTab.getAttribute('aria-selected')).toEqual('false');
        expect(secondNamedTab.getAttribute('aria-selected')).toEqual('false');

        selectNamedTabs("sharedName", clickedId="clicked");

        expect(clickedTab.getAttribute('aria-selected')).toEqual('false');
        expect(firstNamedTab.getAttribute('aria-selected')).toEqual('true');
        expect(secondNamedTab.getAttribute('aria-selected')).toEqual('false');
    })
    it("doesn't change selected tab if tabList does not contain the name", () => {
        document.body.innerHTML =
        '<div>' +
        '  <div>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-1-1" aria-selected="true">Test tab 1</button>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-1-2" aria-selected="false" name="sharedName" id="clicked">Test tab 2</button>' +
        '  </div>' +
        '  <div id="panel-1-1">Test panel 1</div>' +
        '  <div id="panel-1-2" hidden="true"">Test panel 2</div>' +
        '</div>' +
        '<div>' +
        '  <div>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-2-1" aria-selected="true" id="tab-2-1" >Test tab 1</button>' +
        '    <button class="sphinx-tabs-tab" aria-controls="panel-2-2" aria-selected="false" name="aName" id="tab-2-2">Test tab 2</button>' +
        '  </div>' +
        '  <div id="panel-2-1">Test panel 1</div>' +
        '  <div id="panel-2-2" hidden="true">Test panel 2</div>' +
        '</div>';

        let clickedTab = document.getElementById('clicked');
        let tab1Tabset2 = document.getElementById('tab-2-1');
        let tab2Tabset2 = document.getElementById('tab-2-2');
        expect(clickedTab.getAttribute('aria-selected')).toEqual('false');
        expect(tab1Tabset2.getAttribute('aria-selected')).toEqual('true');
        expect(tab2Tabset2.getAttribute('aria-selected')).toEqual('false');

        selectNamedTabs("sharedName", clickedId="clicked");

        expect(clickedTab.getAttribute('aria-selected')).toEqual('false');
        expect(tab1Tabset2.getAttribute('aria-selected')).toEqual('true');
        expect(tab2Tabset2.getAttribute('aria-selected')).toEqual('false');
    })
})
