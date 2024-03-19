// small pieces of code move from main file to reduce its size

#import "@preview/sourcerer:0.2.1"

#let _code(
    source,
    caption,
    kind,
    supplement,
) = {
    assert(caption != "", message: "Provide a caption")

    return figure(
        sourcerer.code(
            source,
            numbering: false,
            fill: rgb("#3F3F3F")
        ),
        caption: caption,
        kind: kind,
        supplement: supplement,
    );
}

#let snippet(source, caption: "") = {
    return _code(source, caption, "snippet", "Código");
}

#let cli(source, caption: "") = {
    // replace "$" with actual input prompt
    let _source = (
        block: source.block,
        lang: source.lang,
        text: source.text.replace("$", "elpekenin@PC:~$"),
    )

    return _code(_source, caption, "cmd", "Comando");
}

#let page_footer = locate(
        loc => {
            let page_number = counter(page).at(loc).last()

            // no numbering in front page
            if page_number == 0 {
                return
            }

            let position = if calc.even(page_number) {
                left
            } else {
                right
            }

            text(8pt)[
                #align(position)[
                    #counter(page).display(
                        both: true,
                        (current, total) => {
                            let text = [Pág. #current / #total]
                            style(
                                style => {
                                    let width = measure(text, style).width * current / total
                                    
                                    block[
                                        #text
                                        #align(left)[
                                            #line(length: width, stroke: rgb(0, 0, 200))
                                        ]
                                    ]
                                }
                            )
                        }
                    )
                ]
            ]
        }
    )