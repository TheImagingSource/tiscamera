const { selectTab } = require("../sphinx_tabs/static/tabs");

describe("selectTab", () => {
    test("unselected tab become selected", () => {
        document.body.innerHTML =
        '<div>' +
        '  <button id="tab" aria-controls="panel" aria-selected="false">Test</button>' +
        '  <div id="panel" hidden="true">Test</div>' +
        '</div>';

        let tab = document.getElementById('tab');
        let panel = document.getElementById('panel');
        expect(tab.getAttribute('aria-selected')).toEqual('false');
        expect(panel.getAttribute('hidden')).toEqual("true");

        selectTab(tab);

        expect(tab.getAttribute('aria-selected')).toEqual('true');
        expect(panel.getAttribute('hidden')).toEqual(null);
    })
    test("selected tab stays selected", () => {
        document.body.innerHTML =
        '<div>' +
        '  <button id="test-button" aria-controls="panel" aria-selected="true">Test</button>' +
        '  <div id="panel">Test</div>' +
        '</div>';

        let tab = document.getElementById('test-button');
        let panel = document.getElementById('panel');
        expect(tab.getAttribute('aria-selected')).toEqual('true');
        expect(panel.getAttribute('hidden')).toEqual(null);

        selectTab(tab);

        expect(tab.getAttribute('aria-selected')).toEqual('true');
        expect(panel.getAttribute('hidden')).toEqual(null);
    })
})
