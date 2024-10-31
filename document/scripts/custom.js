document.addEventListener('DOMContentLoaded', function () {
    var figures = document.querySelectorAll('div.title');
    var tables = document.querySelectorAll('caption.title');

    function updateCaptions(elements, type) {
        var sect_cnt = {};
        elements.forEach(function (element) {
            let before = element.innerText;
            console.log(`${before}`);
            if (!before.match(`^${type} `)) {
                return;
            }
                
            // console.log(element + " " + element.innerText);
            const ancestor = element.closest("div.sect1");
            const sect = ancestor.querySelector('h2');
            const snum = sect.innerText.replace(/\..*$/, '').trim();
            if (!sect_cnt[snum]) {
                sect_cnt[snum] = 0;
            }
            sect_cnt[snum]++;
            const counter = sect_cnt[snum];
            const aft = before.replace(/\d+/, `${snum}.${counter}`);
            element.innerText = aft;
            // console.log(aft);
        });
    }

    updateCaptions(figures, '図');
    updateCaptions(tables, '表');
});
