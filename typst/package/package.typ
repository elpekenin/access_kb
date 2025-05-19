#import "config.typ"
#import "glossary.typ": glossary
#import "images.typ": images
#import "tools.typ": tools

#let __code(
  source,
  caption,
  kind,
  supplement,
) = {
  assert(caption != "", message: "Provide a caption")

  return figure(
    [#source],
    caption: caption,
    kind: kind,
    supplement: supplement,
  )
}

#let snippet(source, caption: "") = {
  return __code(source, caption, "snippet", "Código")
}

#let cli(source, caption: "") = {
  // replace "$" with actual input prompt
  let _source = raw(
    source.text.replace("$", "user@computer:~$"),
    block: source.block,
    lang: source.lang,
  )

  return __code(_source, caption, "cmd", "Comando")
}
