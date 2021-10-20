const { deselectTabList } = require("../sphinx_tabs/static/tabs");

describe("deselectTabList", () => {
  it("deselects all tabs in tablist when one tab is clicked", () => {
    document.body.innerHTML =
    '<div>' +
    '  <div>' +
    '    <button id="tab-1" aria-controls="panel-1" aria-selected="true">Test tab 1</button>' +
    '    <button id="tab-2" aria-controls="panel-2" aria-selected="true">Test tab 2</button>' +
    '  </div>' +
    '  <div id="panel-1">Test panel 1</div>' +
    '  <div id="panel-2">Test panel 2</div>' +
    '</div>';

    let tab1 = document.getElementById('tab-1');
    let tab2 = document.getElementById('tab-2');
    let panel1 = document.getElementById('panel-1');
    let panel2 = document.getElementById('panel-2');
    expect(tab1.getAttribute('aria-selected')).toEqual('true');
    expect(tab2.getAttribute('aria-selected')).toEqual('true');
    expect(panel1.getAttribute('hidden')).toEqual(null);
    expect(panel2.getAttribute('hidden')).toEqual(null);

    deselectTabList(tab1);

    expect(tab1.getAttribute('aria-selected')).toEqual('false');
    expect(tab2.getAttribute('aria-selected')).toEqual('false');
    expect(panel1.getAttribute('hidden')).toEqual('true');
    expect(panel2.getAttribute('hidden')).toEqual('true');
  })
})
