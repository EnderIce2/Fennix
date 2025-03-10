## Admonitions

> :warning: **Warning:** This is a warning.

> :memo: **Note:** This is a note.

> :bulb: **Tip:** This is a tip.

## GitHub Admonitions

> [!NOTE]
> This is a note.

> [!TIP]
> This is a tip.

> [!IMPORTANT]
> This is important.

> [!WARNING]
> This is a warning.

> [!CAUTION]
> This is a caution.

## python-markdown Admonitions

!!! note "This is a note."
	Hello World!

!!! danger "This is a danger."
	Hello World!

!!! important "This is important."
	Hello World!

!!! warning "This is a warning."
	Hello World!

!!! caution "This is a caution."
	Hello World!

[The section below is an excerpt of the original.](https://www.markdownguide.org/cheat-sheet/)
The original content is licensed under a CC BY-SA 4.0 license. The original content can be found at the link above.

## Headings

# Heading level 1
## Heading level 2
### Heading level 3
#### Heading level 4
##### Heading level 5
###### Heading level 6

## Alternate Syntax

Heading level 1
===============

Heading level 2
---------------

---

## Paragraphs

I really like using Markdown.

I think I'll use it to format all of my documents from now on. 

---

## Line Breaks

This is the first line.  
And this is the second line.

---

## Emphasis

I just love **bold text**.

I just love __bold text__.

Love**is**bold

Italicized text is the *cat's meow*.

Italicized text is the _cat's meow_.

A*cat*meow

This text is ***really important***.

This text is ___really important___.

This text is __*really important*__.

This text is **_really important_**.

This is really***very***important text.

---

## Blockquotes

> Dorothy followed her through many of the beautiful rooms in her castle.

> Dorothy followed her through many of the beautiful rooms in her castle.
>
> The Witch bade her clean the pots and kettles and sweep the floor and keep the fire fed with wood.

> Dorothy followed her through many of the beautiful rooms in her castle.
>
>> The Witch bade her clean the pots and kettles and sweep the floor and keep the fire fed with wood.

> #### The quarterly results look great!
>
> - Revenue was off the chart.
> - Profits were higher than ever.
>
>  *Everything* is going according to **plan**.

---

## Lists

List 1

1. First item
2. Second item
3. Third item
4. Fourth item

List 2

1. First item
1. Second item
1. Third item
1. Fourth item

List 3

1. First item
8. Second item
3. Third item
5. Fourth item

List 4

1. First item
2. Second item
3. Third item
    1. Indented item
    2. Indented item
4. Fourth item

List 5

- First item
- Second item
- Third item
- Fourth item

List 6

* First item
* Second item
* Third item
* Fourth item

List 7

+ First item
+ Second item
+ Third item
+ Fourth item

List 8

- First item
- Second item
- Third item
    - Indented item
    - Indented item
- Fourth item

List 9

- 1968\. A great year!
- I think 1969 was second best.

---

## Paragraphs

* This is the first list item.
* Here's the second list item.

    I need to add another paragraph below the second list item.

* And here's the third list item.

* This is the first list item.
* Here's the second list item.

    > A blockquote would look great below the second list item.

* And here's the third list item.

---

## Code Blocks

1. Open the file.
2. Find the following code block on line 21:

        <html>
          <head>
            <title>Test</title>
          </head>

3. Update the title to match the name of your website.

---

## Images

1. Open the file containing the Linux mascot.
2. Marvel at its beauty.

    ![Tux, the Linux mascot](https://mdg.imgix.net/assets/images/tux.png)

3. Close the file.

---

## Code

At the command prompt, type `nano`.

``Use `code` in your Markdown file.``

    <html>
      <head>
      </head>
    </html>

---

## Horizontal Rules

***

---

_________________

---

## Links

My favorite search engine is [Duck Duck Go](https://duckduckgo.com).

My favorite search engine is [Duck Duck Go](https://duckduckgo.com "The best search engine for privacy").

<https://www.markdownguide.org>

<fake@example.com>

I love supporting the **[EFF](https://eff.org)**.

This is the *[Markdown Guide](https://www.markdownguide.org)*.

See the section on [`code`](#code).

In a hole in the ground there lived a hobbit. Not a nasty, dirty, wet hole, filled with the ends
of worms and an oozy smell, nor yet a dry, bare, sandy hole with nothing in it to sit down on or to
eat: it was a [hobbit-hole][1], and that means comfort.

[1]: <https://en.wikipedia.org/wiki/Hobbit#Lifestyle> "Hobbit lifestyles"

---

## Escaping Characters

\* Without the backslash, this would be a bullet in an unordered list.

\\ test

\` test

\* test

\_ test

\{ test

\} test

\[ test

\] test

\< test

\> test

\( test

\) test

\# test

\+ test

\- test

\. test

\! test

\| test

---

## HTML

This **word** is bold. This <em>word</em> is italic.

## Tables

| Syntax      | Description |
| ----------- | ----------- |
| Header      | Title       |
| Paragraph   | Text        |

| Syntax | Description |
| --- | ----------- |
| Header | Title |
| Paragraph | Text |

| Syntax      | Description | Test Text     |
| :---        |    :----:   |          ---: |
| Header      | Title       | Here's this   |
| Paragraph   | Text        | And more      |

---

## Fenced Code Blocks

```
{
  "firstName": "John",
  "lastName": "Smith",
  "age": 25
}
```

```json
{
  "firstName": "John",
  "lastName": "Smith",
  "age": 25
}
```

---

## Footnotes

Here's a simple footnote,[^1] and here's a longer one.[^bignote]

[^1]: This is the first footnote.

[^bignote]: Here's one with multiple paragraphs and code.

    Indent paragraphs to include them in the footnote.

    `{ my code }`

    Add as many paragraphs as you like.

---

## Heading IDs

# Heading level 1 {#custom-id}
## Heading level 2 {#custom-id}
### Heading level 3 {#custom-id}
#### Heading level 4 {#custom-id}
##### Heading level 5 {#custom-id}
###### Heading level 6 {#custom-id}

---

## Strikethrough

~~The world is flat.~~ We now know that the world is round.

---

## Task Lists

- [x] Write the press release
- [ ] Update the website
- [ ] Contact the media

---

## Emoji

Gone camping! :tent: Be back soon.

That is so funny! :joy:

---

## Highlight

I need to highlight these ==very important words==.

---

## Subscript and Superscript

H~2~O

X^2^

---

## Disabling Automatic URL Linking

`http://www.example.com`
