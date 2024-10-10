// small pieces of code move from main file to reduce its size

#let _code(
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
  return _code(source, caption, "snippet", "Código")
}

#let cli(source, caption: "") = {
  // replace "$" with actual input prompt
  let _source = raw(
    source.text.replace("$", "elpekenin@PC:~$"),
    block: source.block,
    lang: source.lang,
  )

  return _code(_source, caption, "cmd", "Comando")
}
