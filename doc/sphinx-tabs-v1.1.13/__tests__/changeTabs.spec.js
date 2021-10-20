const { changeTabs } = require("../sphinx_tabs/static/tabs");
global.scrollTo = jest.fn();

describe("changeTabs", () => {
    it("changes tab when new tab is selected", () => {
        document.body.innerHTML =
        '<div>' +
        '  <div>' +
        '    <button id="tab-1" aria-controls="panel-1" aria-selected="true">Test tab 1</button>' +
        '    <button id="tab-2" aria-controls="panel-2" aria-selected="false">Test tab 2</button>' +
        '  </div>' +
        '  <div id="panel-1">Test panel 1</div>' +
        '  <div id="panel-2" hidden="true">Test panel 2</div>' +
        '</div>';

        const allTabs = document.querySelectorAll('button');
        allTabs.forEach(tab => {
            tab.addEventListener("click", changeTabs);
        });

        let tab1 = document.getElementById('tab-1');
        let tab2 = document.getElementById('tab-2');
        let panel1 = document.getElementById('panel-1');
        let panel2 = document.getElementById('panel-2');
        expect(tab1.getAttribute('aria-selected')).toEqual('true');
        expect(tab2.getAttribute('aria-selected')).toEqual('false');
        expect(panel1.getAttribute('hidden')).toEqual(null);
        expect(panel2.getAttribute('hidden')).toEqual('true');

        tab2.click()

        expect(tab1.getAttribute('aria-selected')).toEqual('false');
        expect(tab2.getAttribute('aria-selected')).toEqual('true');
        expect(panel1.getAttribute('hidden')).toEqual('true');
        expect(panel2.getAttribute('hidden')).toEqual(null);

        tab1.click();

        expect(tab1.getAttribute('aria-selected')).toEqual('true');
        expect(tab2.getAttribute('aria-selected')).toEqual('false');
        expect(panel1.getAttribute('hidden')).toEqual(null);
        expect(panel2.getAttribute('hidden')).toEqual('true');
    })
    it("closes tab in closeable tabList when selected", () => {
        document.body.innerHTML =
        '<div>' +
        '  <div class="closeable">' +
        '    <button id="tab-1" aria-controls="panel-1" aria-selected="true">Test tab 1</button>' +
        '  </div>' +
        '  <div id="panel-1">Test panel 1</div>' +
        '</div>';

        let tab = document.getElementById('tab-1');
        let panel = document.getElementById('panel-1');
        tab.addEventListener("click", changeTabs);

        expect(tab.getAttribute('aria-selected')).toEqual('true');
        expect(panel.getAttribute('hidden')).toEqual(null);

        // Close
        tab.click();

        expect(tab.getAttribute('aria-selected')).toEqual('false');
        expect(panel.getAttribute('hidden')).toEqual('true');

        // Re-open
        tab.click();

        expect(tab.getAttribute('aria-selected')).toEqual('true');
        expect(panel.getAttribute('hidden')).toEqual(null);
    })
    it("does not close tab in non-closeable tabList when selected", () => {
        document.body.innerHTML =
        '<div>' +
        '  <div>' +
        '    <button id="tab-1" aria-controls="panel-1" aria-selected="true">Test tab 1</button>' +
        '  </div>' +
        '  <div id="panel-1">Test panel 1</div>' +
        '</div>';

        let tab = document.getElementById('tab-1');
        let panel = document.getElementById('panel-1');
        tab.addEventListener("click", changeTabs);

        expect(tab.getAttribute('aria-selected')).toEqual('true');
        expect(panel.getAttribute('hidden')).toEqual(null);

        tab.click();

        expect(tab.getAttribute('aria-selected')).toEqual('true');
        expect(panel.getAttribute('hidden')).toEqual(null);
    })
    it("changes tab when content nested inside the tab is clicked", () => {
        document.body.innerHTML =
        '<div>' +
        '  <div>' +
        '    <button id="tab-1" aria-controls="panel-1" aria-selected="true">Test tab 1</button>' +
        '    <button id="tab-2" aria-controls="panel-2" aria-selected="false"><strong id=bold>Test tab 2</strong></button>' +
        '  </div>' +
        '  <div id="panel-1">Test panel 1</div>' +
        '  <div id="panel-2" hidden="true">Test panel 2</div>' +
        '</div>';

        let tab1 = document.getElementById('tab-1');
        let tab2 = document.getElementById('tab-2');
        let panel1 = document.getElementById('panel-1');
        let panel2 = document.getElementById('panel-2');
        expect(tab1.getAttribute('aria-selected')).toEqual('true');
        expect(tab2.getAttribute('aria-selected')).toEqual('false');
        expect(panel1.getAttribute('hidden')).toEqual(null);
        expect(panel2.getAttribute('hidden')).toEqual('true');

        let tab_2_text = document.getElementById('bold');
        expect(tab_2_text.getAttribute('hidden')).toEqual(null);

        tab2.addEventListener("click", changeTabs);
        tab_2_text.click();

        expect(tab1.getAttribute('aria-selected')).toEqual('false');
        expect(tab2.getAttribute('aria-selected')).toEqual('true');
        expect(panel1.getAttribute('hidden')).toEqual('true');
        expect(panel2.getAttribute('hidden')).toEqual(null);

        expect(tab_2_text.getAttribute('hidden')).toEqual(null);
    })
});
