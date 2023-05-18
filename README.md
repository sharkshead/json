# json
A command that turns unreadable JSON, as produced by many commands and APIs these daya,
into a much more human-friendly format.

Usage falls into a couple of patterns.

1. Pipe JSON in as standard input

  ugly-json-producer | json

2. From a file

  json ugly.json

You can output portions of the input by including the name
of the objects as arguments.
Use a - as the file name to read standard input.

3. Selecting the objects to output

  ugly-json-producer | json - author.firstname author.surname

  json ugly.json author.firstname author.surname

You can access arrays in two different ways: explicit reference
or all elements.

3. Accessing array elemens explicitly

  ugly-json-producer | json - 'author.books[0].title'

4. Accessing all elements of an array

  ugly-json-producer | json - 'author.books[].title'

