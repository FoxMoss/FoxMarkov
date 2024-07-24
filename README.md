# Foxkov
Foxkov is a [Markov chain](https://en.wikipedia.org/wiki/Markov_chain) toolset for natural language. By default it is primed to use exports from [DiscordChatExporter](https://github.com/Tyrrrz/DiscordChatExporter)(I am not affiliated with their political beliefs it just works.) but configuration is available.

Check out [my blog](http://foxmoss.com/blog/foxkov.html) on Markov chains.

## Usage Examples

How to extract a user from a dataset:
```
foxkov --csv aBunchOfUsers.csv cleanup --output me.csv --matching-column author --matching-data "myUsername"
```

Find out who's messages matches your Markov chain the most:
```
foxkov --csv me.csv stenography --overview-log overview.csv ./friends/*.csv
```

Generate a message:
```
foxkov --csv me.csv generate
```

