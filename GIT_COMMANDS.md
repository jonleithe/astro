# Git Terminal Stats Quick Guide

These commands give a broader status view than `git status` alone.

## Branch and Tracking Overview

- `git branch -vv`
  Shows local branches, upstream branch, and ahead/behind counters.

## Remote URLs

- `git remote -v`
  Shows fetch/push URLs for all configured remotes.

## Detailed Remote State

- `git remote show origin`
  Shows remote HEAD branch, tracked branches, and pull/push configuration.

## Compact Working Tree Status

- `git status -sb`
  Short status with branch and tracking summary.

## Change Size Summary

- `git diff --stat`
  Shows unstaged file change counts.

- `git diff --stat --cached`
  Shows staged file change counts.

## Commit Graph View

- `git log --oneline --graph --decorate --all -n 25`
  Compact graph of recent local and remote history.

## Verify Remote Branch Hash

- `git ls-remote --heads origin main`
  Shows commit hash currently at remote `main`.

## One-liner Snapshot

```bash
git remote -v && echo && git branch -vv && echo && git status -sb
```
