"use client";

import { CheckCheckIcon, Copy } from "lucide-react";
import { useState } from "react";

export default function CopyButton({ code }: { code: string }) {
  const [copied, setCopied] = useState(false);

  const handleCopy = async () => {
    await navigator.clipboard.writeText(code);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  return (
    <button
      onClick={handleCopy}
      title="Copiar código"
      className="text-white cursor-pointer"
    >
      {copied ? <CheckCheckIcon /> : <Copy />}
    </button>
  );
}
