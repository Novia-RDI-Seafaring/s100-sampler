# Release scope and remaining findings

The November 2026 audit separates publishing source from distributing a Windows
research preview and from claiming a reliable live VTS exchange. The source is a
research/integration client; it is not an operational navigation or clearance
system.

## Findings addressed in this change

- **B1:** downloaded dataset identifiers are constrained to safe leaf filenames.
  Parent segments, path separators, Windows device/stream forms and existing
  symlink destinations are rejected. Regression tests exercise the real saving
  function using disposable files.
- **R1:** the SECOMLib patch records its upstream revisions and Apache-2.0 scope.
  The full licence is included; the original project code remains MIT.
- **B2:** Windows packaging includes dependency notices, LGPL/GPL/Apache texts,
  and verified corresponding sources. Packaging fails when the source archive or
  installed dependency records do not match. See [Releasing](RELEASING.md).
- **R5:** XML validation resolves `xmllint` beside the executable and then on
  PATH. The Windows distribution deploys the tool and its non-system DLLs.
  A synthetic XML/XSD test verifies this with PATH empty.

## Remaining correctness findings

The disposition below is specific to this research release. Documentation does
not establish that a defect is fixed. These findings remain in the implementation.

| Finding | Disposition for source / offline research preview | Required before broader functional claims |
| --- | --- | --- |
| Remote refresh clears the published dataset before successful retrieval; backup failures are ignored | Documented limitation. Keep independent copies; offline reload avoids remote retrieval | Fix before claiming reliable remote refresh or preservation of the last successful snapshot |
| Unresolved/prefixed AtoN status references can reach a null dereference | Documented input-support limitation; review S-125 inputs before loading | Fix and test before claiming general S-125 input support |
| Selecting a record without parts/geometry can index empty collections | Documented limitation; avoid selecting such rows | Fix before claiming support for geometryless/cancellation warning selection |
| VTS tab position is used as thread identity; closing tabs can redirect actions | Source publication is permitted with this disclosed defect; no reliable live VTS claim | **Fix before a live VTS release** |
| Duplicate backend uploads can replace accepted content before a unique insert fails | Backend is absent from the offline Windows package; source defect is disclosed | **Fix before externally deploying the receiving backend**; test identical/conflicting retries |
| Backend acceptance checks run after persistence | Same backend limitation | **Fix before externally deploying the receiving backend**; rejected input must not be published |
| Rejected HTTP sends/ACKs can be recorded as successful locally | Experimental VTS path remains disclosed | **Fix before a live VTS release**; retain rejected operations for correction/retry |
| VTS polling can overlap; worker lifetime is not joined at shutdown | Avoid activating S-212 for the offline preview | **Fix before a live VTS release**; include slow-network and shutdown tests |
| VTS deactivation clears the selected service before the guarded endpoint reset | Same offline scope; deactivation does not prove that the endpoint was cleared | **Fix before a live VTS release**, together with in-flight worker handling |
| Registry rows match organization/name while service identity also includes endpoint/product | Documented limitation for duplicate service names; verify endpoint/product independently | Fix before live-service use with ambiguous registry rows |
| Backend ACK storage unconditionally replaces the previous state for a transaction | Source behavior is disclosed; monotonic delivery state is not claimed | Resolve endpoint ordering semantics and fix/test delayed ACKs before a reliable live VTS status claim |
| NAVWARN nested validity ranges are skipped; truncated-date day and time offsets are parsed incorrectly | File inspection exposes only a subset of fields; validity/time correctness is not claimed | Fix before relying on validity windows or cross-timezone ordering |
| Undownloaded backend routes mark receipt before reading files successfully | Desktop UUID polling does not use the affected queue route | Fix before offering successful-delivery/queue retry guarantees on those routes |

Unsigned placeholder message signatures, a user-supplied schema, and the
loopback-only backend profile remain documented integration limits. Full S-212
interoperability with an external VTS service still needs independent validation.
The new focused tests do not establish that workflow.

The prior review also identified unresolved ownership: clearing raw feature-map
pointers does not release the models, and parser-supplied status pointers are
adopted by `ATONMember`'s `unique_ptr`. Shared references therefore need an explicit
ownership decision before repeated/long-running reloads are supported. No leak,
double-free or race diagnostics were run in this change; this remains an
unresolved risk rather than a newly reproduced crash.

## Publication decisions

- **Public source visibility:** GO for the reviewed fixes and disclosed research
  scope. The confidentiality/history audit found no real credentials or restricted
  datasets in the audited Git material. Personal author email metadata remains in
  history. Hosted security scanning was unavailable/disabled during that audit.
- **Offline Windows research preview:** GO only for a freshly built, verified
  package retaining its notices and corresponding source archive. The old draft
  ZIP is superseded by the rebuilt artifact; it is not corrected in place on
  GitHub by a local build.
- **Reliable live VTS/backend distribution or deployment:** NO-GO until the
  explicitly marked findings above are fixed and the exchange is validated.

The existing `v1.0.0` tag and GitHub draft are unchanged. Publishing source from
the old tag/main does not include these fixes. The release owner must select a
reviewed commit and matching assets together; [Releasing](RELEASING.md) describes
the version/tag decision. No visibility change or publication is part of the
local implementation and validation work.
