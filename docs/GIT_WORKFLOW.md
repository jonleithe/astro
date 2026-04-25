# Git Workflow Notes

[Back to README](../README.md)

Related docs:

- [Build and run](BUILD.md)
- [API reference](API.md)
- [Project structure](PROJECT_STRUCTURE.md)

This file mirrors and extends the quick helper commands in GIT_COMMANDS.md.

## Quick Status Overview

```bash
git remote -v && echo && git branch -vv && echo && git status -sb
```

## Useful Details

Branch/upstream tracking:

```bash
git branch -vv
```

Remote configuration:

```bash
git remote -v
git remote show origin
```

Change size summary:

```bash
git diff --stat
git diff --stat --cached
```

Recent graph:

```bash
git log --oneline --graph --decorate --all -n 25
```

## SSH Remote Pattern

If using a host alias in ~/.ssh/config (example: github-jonleithe), origin can be set to:

```bash
git remote set-url origin git@github-jonleithe:jonleithe/astro.git
```

Then push with:

```bash
git push
```
