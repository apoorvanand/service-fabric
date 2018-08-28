// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace System.Fabric.Description
{
    internal enum NodeTask : byte
    {
        Invalid = 0,
        Restart,
        Relocate,
        Remove
    }
}