#import "vars.typ"
#import "glossary.typ": glossary
#import "images.typ": images

#let __code(
  source,
  caption,
  kind,
  supplement,
) = {
  assert(caption != "", message: "Provide a caption")

  return figure(
    source,
    caption: caption,
    kind: kind,
    supplement: supplement,
  )
}

#let snippet(source, caption: "", size: 11pt) = {
  return __code(
    text(
      source,
      size: size,
    ),
    caption,
    "snippet",
    "Código",
  )
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

#let alignment(value) = {
  return if calc.even(value) { left } else { right }
}

#let nest-level = state("nest-level", 0)

#let h(title, body) = context {
  nest-level.update(level => level + 1)
  let level = nest-level.get() + 1

  heading(level: level)[#title]
  body

  nest-level.update(level => level - 1)
}
