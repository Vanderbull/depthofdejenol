# Working with Milestones and Versions

Status: Draft  
Last updated: 2026-01-24

## Purpose
This document explains how to use milestones and versions together to plan, track, and ship releases. It covers definitions, naming conventions, recommended workflows, practical CLI examples, templates for milestone and release descriptions, and automation options to make releases repeatable and low-friction.

---

## Key concepts

- Milestone
  - A GitHub construct (under Issues) used to group issues and pull requests that should be completed for a particular target (often a release).
  - Best used for planning, scope tracking, and progress visibility.

- Version / Tag / Release
  - A Git tag (for example `v1.2.0`) marks the code state for a release.
  - A GitHub Release is a higher-level object usually associated with a tag and release notes/artifacts.
  - "Version" typically refers to the public version number (SemVer).

- Relationship
  - Use milestones to plan and finish work for a version. When a milestone is complete (all critical issues/PRs merged), create a tag and a GitHub Release for the associated version.

---

## Recommended high-level workflow

1. Create a milestone for the target version (e.g., `v1.2.0`).
2. Add issues and PRs to the milestone as scope is decided.
3. Implement work on feature branches; open PRs and link them to the milestone.
4. Merge PRs into the mainline branch (e.g., `main`) when ready, keeping the milestone up to date.
5. When the milestone scope is complete and tests pass:
   - Create a release candidate branch if needed (e.g., `release/v1.2.0-rc`).
   - Run final QA.
   - Create an annotated tag `v1.2.0`.
   - Publish a GitHub Release with release notes (use generated changelog if available).
6. Close the milestone (optional if milestones are closed automatically by tagging/releasing).
7. Publish artifacts and notify stakeholders.

---

## Naming conventions

Prefer a predictable convention. Examples:
- Milestones: `v1.2.0`, `v2.0.0-rc1`, or `1.2.0 (Feb 2026)` depending on preference.
- Tags/Releases: `vMAJOR.MINOR.PATCH` (e.g., `v1.2.0`) — match the milestone exactly if possible.
- Branches: `release/v1.2.0`, `hotfix/v1.2.1`, `feature/<short-desc>`.

SemVer recap:
- Increment MAJOR for incompatible API changes.
- Increment MINOR for added functionality in a backwards-compatible manner.
- Increment PATCH for backwards-compatible bug fixes.

---

## Creating milestones

UI:
- Issues → Milestones → New milestone
  - Title: `v1.2.0`
  - Due date: optional
  - Description: include goals, scope, and acceptance criteria

CLI (GitHub CLI - `gh`):
- Create:
  - gh milestone create "v1.2.0" --due "2026-02-28" --description "Goals: ...\nScope: ...\nAcceptance criteria: ..."
- Assign an existing issue to a milestone:
  - gh issue edit 123 --milestone "v1.2.0"

API (curl example):
- Create milestone (replace OWNER/REPO and provide a token):
  - curl -X POST -H "Authorization: token $GITHUB_TOKEN" \
    -d '{"title":"v1.2.0","due_on":"2026-02-28T23:59:59Z","description":"Goals..."}' \
    https://api.github.com/repos/OWNER/REPO/milestones

Best practice:
- Keep milestones focused (one per release).
- Use milestones to indicate a realistic, testable scope rather than a wish list.

---

## Linking issues and PRs to milestones

- UI: In an issue or PR sidebar, set the milestone.
- CLI: gh issue edit <number> --milestone "v1.2.0"
- PR templates or issue templates can prompt authors to propose a milestone.
- Use labels (e.g., `target:v1.2.0`) if you want an additional cross-check layer.

---

## From milestone to release (manual steps)

1. Verify milestone completeness:
   - Ensure required issues/PRs are merged or triaged.
   - Resolve or document any deferred items.

2. Create a tag and push:
   - git checkout main
   - git pull origin main
   - git tag -a v1.2.0 -m "Release v1.2.0"
   - git push origin v1.2.0

3. Create a GitHub Release (CLI example):
   - gh release create v1.2.0 --title "v1.2.0" --notes-file RELEASE_NOTES.md

4. Close the milestone in GitHub (optional if you rely on automation).

Release notes content:
- Summary of changes
- Notable bug fixes and feature highlights
- Upgrade notes or breaking changes
- Links to issues/PRs (GitHub/auto tools can generate this)

---

## Automation and recommended tools

- release-drafter
  - Dynamically drafts a release from merged PRs and labels — useful for keeping draft releases up to date.

- semantic-release
  - Automates versioning and release creation from commit messages following Conventional Commits.

- GitHub Actions
  - Use Actions to:
    - Update a draft release as PRs merge.
    - Generate a changelog from merged PRs.
    - Create the final release when a milestone is closed or when a tag is pushed.

Example: simple workflow to create a release when a tag is pushed
```yaml
name: Create Release on Tag
on:
  push:
    tags:
      - 'v*.*.*'
jobs:
  create_release:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Create GitHub Release
        uses: softprops/action-gh-release@v1
        with:
          name: ${{ github.ref_name }}
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
```

Note: If you want a workflow triggered by milestone closure, you can:
- Use a small webhook receiver or GitHub App to respond to `milestone` events, or
- Use Actions that run manually or on a label/issue event that you apply as part of closing the milestone.
- Many teams prefer using a manual "Create release" workflow (workflow_dispatch) to gather QA signoff before tag+release.

---

## Changelog generation

Options:
- Conventional commits + commit analyzer tools (semantic-release).
- Use `git log` or `github_changelog_generator` for simple lists.
- Use release-drafter to maintain a draft release that collects merged PRs.

Example (generate simple changelog from git):
- git log --pretty=format:"- %s (%h)" v1.1.0..v1.2.0 > changelog.md

---

## Release checklist (template)

- [ ] Milestone scope finalized and issues linked
- [ ] All critical PRs merged into `main` (or release branch)
- [ ] CI green for `main` / release branch
- [ ] Release candidate tested (if applicable)
- [ ] Bump version in source/metadata (if your project needs it)
- [ ] Create annotated tag `vX.Y.Z`
- [ ] Create GitHub Release with release notes and artifacts
- [ ] Publish artifacts (packages, installers)
- [ ] Update docs and notify stakeholders
- [ ] Close milestone

---

## Milestone description template

```
Title: vX.Y.Z

Goals:
- Short list of what this release aims to deliver

Scope:
- Feature A
- Improvement B
- Bugfix C

Acceptance criteria:
- Tests pass
- Performance regression check
- Upgrade steps documented

Target date: YYYY-MM-DD
Release owner: @username
```

---

## Release notes template

- Summary of the release
- Notable features
- Bug fixes
- Breaking changes and migration steps
- Contributors and references (list relevant PR/issue links)

---

## Examples: CLI commands (quick reference)

- Create milestone:
  - gh milestone create "v1.2.0" --due "2026-02-28" --description "Milestone description"
- Assign issue:
  - gh issue edit 123 --milestone "v1.2.0"
- Create tag:
  - git tag -a v1.2.0 -m "Release v1.2.0"
  - git push origin v1.2.0
- Create release:
  - gh release create v1.2.0 --title "v1.2.0" --notes-file RELEASE_NOTES.md

---

## Policies and best practices

- One release per milestone: keep clarity between planning (milestone) and shipped artifact (tag/release).
- Keep milestones small and achievable — prefer multiple small milestones over a single huge one.
- Link work to milestones early to avoid last-minute scope creep.
- Automate what you can (draft releases, changelog) but keep manual checkpoints for QA and signoff.
- Use labels (e.g., `breaking-change`, `docs`, `security`) to help generate meaningful release notes.

---

## Advanced patterns

- Release branches for long stabilization (e.g., `release/v1.2.0`).
- Hotfixes: branch from latest release tag, merge back to `main`, create patch release.
- Canary/preview releases: use pre-release tags (e.g., `v1.3.0-rc.1`) and mark them as pre-release on GitHub.

---

## Troubleshooting and FAQs

Q: Should I close a milestone before or after creating the release?
A: Either is fine, but many teams close the milestone after the release is published so the milestone reflects the closed state of planning and shipping.

Q: How do I include PR links in release notes?
A: Use tools (release-drafter, semantic-release) that auto-populate PRs, or gather merged PRs between tags using the GitHub API or `git log`.

Q: How can I automate releases from milestones?
A: Combine labeling, release-drafter, and a workflow that runs on tag push or a manual `workflow_dispatch` that reads the milestone and creates the final release via the API or `gh` CLI.

---

## Next steps (suggested)

- Agree on naming conventions (milestone vs tag) for your team.
- Add the milestone and release templates to `.github/ISSUE_TEMPLATE` or `.github/` docs folder.
- Add a release automation workflow (draft) using `release-drafter` or a tag-triggered workflow.
- If you want, I can:
  - Create this file in a repository you choose,
  - Draft a GitHub Actions workflow (more specific to your repo),
  - Or adapt the templates to your team's format.

---

Appendix: Useful links
- GitHub Milestones docs: https://docs.github.com/en/issues/organizing-your-work-with-issues-and-projects/managing-your-work-with-issues/about-milestones
- GitHub Releases docs: https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases
- GitHub CLI: https://cli.github.com/
- release-drafter: https://github.com/release-drafter/release-drafter
- semantic-release: https://github.com/semantic-release/semantic-release